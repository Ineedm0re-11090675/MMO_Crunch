# MMO_Crunch 接入 Maple：从 Input 开始的重构路线

> 2026-10-08，依据当前本机源码整理。本文是实施指南，没有修改项目 C++、蓝图或配置。插件源码可读取；本会话没有可调用的 VibeUE 接口，因此蓝图父类、组件配置和 Data Asset 字段仍要在 UE 编辑器中核对。

## 先给出三个决定

1. **可以把玩家输入的入口放到 `ACPlayerController`，但不要把所有游戏行为都写进 Controller。** 建议由 Controller 持有一个 `UMapleInputManagerComponent`，由 `UMapleInputHandler` 处理各个 Action；Handler 通过当前 Pawn、ASC 或接口调用实际行为。这样换角色时 Controller 不必换，角色仍负责移动、技能状态、相机和背包等自身能力。
2. **先只迁移 Input，暂时不改角色继承链。** 当前工程还没有完成 MapleGAS 的 ASC 迁移。输入能否收到、能否找到正确 ASC、技能能否激活是三个不同的验收点。把它们一次改完，很难判断失败来自哪一层。
3. **角色最终采用两个分支，不做双重 UObject 继承。** 现有 `ACPlayerCharacter : ACCharacter : AMapleGASCharacter`，Maple 的 `AMapleGASPlayerCharacter : AMapleGASCharacter`。`ACPlayerCharacter` 不能同时继承 `ACCharacter` 和 `AMapleGASPlayerCharacter`；二者已经各自经过 `ACharacter`，而 UE 的 `UCLASS` 也不支持把两个 Actor 类当作多个基类。共用逻辑应抽到组件、接口或普通辅助对象。

## 1. 现在有哪些东西

项目源代码实际在 `/Users/sunthemac/UnrealProjects/MMO_Crunch`。三个插件的可读源码在 `/Users/sunthemac/UnrealProjects/MaplePlugins`，当前 `MMO_Crunch.uproject` 已启用三个插件，`MMO_Crunch.Build.cs` 已声明其模块依赖。插件仓库中版本分别为 MapleInput `5.6.14`、MapleGAS `1.10.12`、MapleCombat `7.0.0`；实施前应确认编辑器实际加载的是哪份插件副本。

| 位置 | 当前职责/状态 | 重构时的含义 |
| --- | --- | --- |
| [ACPlayerController::SetupInputComponent](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerController.cpp:45) | 添加 UI Mapping Context，绑定商店和菜单 Action | 这是玩家级输入入口；迁移时避免与 Maple 再绑定同一 Action |
| [ACPlayerCharacter::SetupPlayerInputComponent](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerCharacter.cpp:52) | 整段绑定代码目前被注释 | 不能把这些旧绑定当作当前运行行为；先在编辑器确认蓝图是否另有绑定 |
| [ACPlayerCharacter::HandleAbilityInput](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerCharacter.cpp:108) | 设计上把按下/松开传给 ASC，基础攻击还发送 Gameplay Event | 它是迁移时的行为清单，尤其不能漏掉松开和连招事件 |
| [UInputHandler_AbilityInputPlus](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Input/Handlers/InputHandler_AbilityInputPlus.cpp:8) | 项目新增的按 `ECAbilityInputId` 查找、按下技能 Handler | 目前仍在工作树中修改中；应先用一个技能验证它的实际事件序列 |
| [UInputHandler_Shoot](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Input/Handlers/InputHandler_Shoot.h:12) | 只有类声明，尚无处理逻辑 | 不列入“已接通射击”的验收 |
| [ACCharacter::GetAbilitySystemComponent](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Character/CCharacter.cpp:88) | 返回项目旧 `UCAbilitySystemComponent` | MapleInput 通过 Pawn 查 ASC 时，当前会拿到这个旧 ASC |
| [ACCharacter::ACCharacter](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Character/CCharacter.cpp:36) | 已继承 `AMapleGASCharacter`，又创建旧 ASC | 角色上存在两套 ASC 路径；迁移前必须记录组件和蓝图引用 |
| [ACPlayerState](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerState.h:14) | 仍直接继承 `APlayerState` | 若采用 Maple 玩家角色，其 ASC 应从 Maple PlayerState 路径取得 |

已找到 `IS_Crunch.uasset`、`DA_CrunchGAS.uasset`、`BP_AI.uasset`、`BP_Player.uasset` 等资产文件。**文件存在不代表**它们的父类、InputHandlers、IMC、ASC 或属性配置正确；编辑器检查记录见第 7 节。

## 2. 先理解 MapleInput 的一条完整链

```text
按键/鼠标
  → Input Mapping Context (IMC) 中的 Input Action (IA)
  → UMapleInputManagerComponent 绑定并分发 Started/Triggered/Completed 等事件
  → UMapleInputSetupDataAsset 中配置的 UMapleInputHandler
  → Handler 读取 Manager 的当前 Pawn / Controller / ASC
  → Pawn 移动、相机转向、GameplayAbility、UI 等实际行为
```

阅读顺序建议如下，每读完一项就回答右边的问题：

| 顺序 | 源码入口 | 要弄懂的问题 |
| --- | --- | --- |
| 1 | [UMapleInputHandler](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Public/MapleInputHandler.h:17) | `InputActions` 与 `TriggerEvents` 怎样共同决定 Handler 是否接收事件？`Started`、`Triggered`、`Completed` 分别进哪个函数？ |
| 2 | [UMapleInputSetupDataAsset](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Public/Data/MapleInputSetupDataAsset.h:15) | 一个 Setup 如何组合 IMC、优先级和 Handler？ |
| 3 | [UMapleInputManagerComponent::AddInputSetupData](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:383) | 重复 Setup 或重复 IMC 会怎样？没有匹配的绑定会怎样？ |
| 4 | [UMapleInputManagerComponent::Dispatch](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:552) | 输入被关闭或 Buffer 打开后会发生什么？ |
| 5 | [UInputHandler_Move](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Input/Handlers/InputHandler_Move.cpp:29)、[UInputHandler_Look](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Input/Handlers/InputHandler_Look.cpp:33) | Handler 怎样通过 `Manager->GetPawn()` 操作角色？ |

Manager 的 [BeginPlay/Controller 初始化](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:56) 支持组件挂在 Pawn 或 PlayerController；[OnPossessedPawnChanged](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:132) 会移除旧 Pawn 的 Setup，读取新 Pawn 的 `IInputSetupProviderInterface`。因此本项目建议 **Controller 持有 Manager，Pawn 只提供自己专属的 Setup**。这里的“提供”可以先在蓝图实现接口，待方案稳定再决定是否改成 C++。

注意：Manager 头文件注释提到了 PlayerState，但当前 [GetPawn 实现](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:1164) 只处理 Pawn/Controller Owner。本项目不把 Manager 放在 PlayerState。

## 3. Controller、Handler、Character 如何分工

| 对象 | 应负责 | 不应接手的内容 |
| --- | --- | --- |
| `ACPlayerController` | 拥有本地输入管理组件；管理 UI/Gameplay 模式、菜单、当前 Pawn 切换和玩家级输入配置 | 技能的授予、血蓝数值、死亡表现、所有角色各自的移动细节 |
| `UMapleInputManagerComponent` | 装载 Setup、管理 IMC、绑定/分发 Action、控制输入是否处理 | 每个技能的项目规则 |
| `UMapleInputHandler` 或项目子类 | 一个明确的输入意图，如移动、技能按下、背包槽位、打开商店 | 长期保存玩家状态或强行持有旧 Pawn 指针 |
| Pawn/Character | 执行移动、相机、背包、动画、死亡等；通过接口提供角色专属 Input Setup | 重复注册已经由 Controller/Manager 管理的同一批按键 |
| PlayerState/ASC | 保存玩家 GAS 状态和技能；Pawn 是 Avatar | 直接读取键盘或创建 IMC |

**推荐按“输入意图”分层，而不是按类名搬代码。** 例如 `Move Handler → 当前 Pawn->AddMovementInput`；`Shop Handler → 当前 Controller 的 UI 函数`；`Ability Handler → 当前 Pawn 的 ASC`。Controller 可以成为统一的持有者，但不需要成为所有行为的实现者。

### 需要特别看清的生命周期

- Controller 在重生、换英雄、Seamless Travel 后可能继续存在，Pawn 可能更换。不要在 Handler 内缓存一个永远不变的 `ACPlayerCharacter*`；每次从 Manager 取当前 Pawn。
- 当前 Manager 默认 `bResetSetupForNewPawn = true`，换 Pawn 时会清理并重建配置。[构造函数](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:30) 要用“换 Pawn 后仍有 UI 输入、没有旧角色技能绑定”验证这一点。
- `GetAbilitySystemComponent()` 从 **当前 Pawn** 查询 GAS 接口。[源码](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:919) 这使 Input 与之后的 MapleGAS 迁移相连：输入先能被分发，不等于已经取到目标 ASC。
- Maple 的移动 Handler 默认使用 Manager 的朝向；没有 ForwardReference 时使用 Controller 水平朝向。[源码](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:1096) 当前项目旧移动方向从 Camera 向量计算，迁移时要用“相机转 90° 后按前/右”的操作比对手感。

## 4. 第一轮只做一个小实验

目标：**弄清 MapleInput 的事件链**，不碰角色继承、不碰技能系统、不改正式地图。

1. 在独立的调试地图或调试 GameMode 中使用当前玩家 Pawn。记录当前地图、GameMode、PlayerController、Pawn 蓝图的父类和组件。
2. 在编辑器确认 Controller 上是否已经有 `MapleInputManagerComponent`；如果没有，实施时只在调试 Controller 蓝图添加一个。不要同时在 Pawn 再放第二个 Manager。
3. 建一个只用于实验的 `UMapleInputSetupDataAsset`，使用一个测试 IMC、一个 Action、一个 `ShowDebugString` Handler。先观察 `Started → Triggered → Completed` 的实际顺序；不用技能作为第一项实验，因为技能失败还可能是 ASC、InputID 或 Ability 的问题。
4. 把同一实验改为移动或跳跃 Handler，确认 `Manager->GetPawn()` 是当前角色；换 Pawn/重生后再次确认。随后关闭调试 Handler。
5. 记录每一步：按键、收到的 TriggerEvent、Manager Owner、当前 Pawn、结果。任何一步没有证据，就停在该步定位，不继续迁移下一层。

**完成标志：** 本地玩家只收到一次输入；换 Pawn 后指向新 Pawn；监听服务器和一个客户端都能移动/跳跃；旧 Pawn 的 Setup 没有遗留。因为本会话不能检查资产内容，这一步的具体蓝图设置须以编辑器实际字段为准。

## 5. 第二轮迁移项目输入：每次只迁一类 Action

建议顺序：`移动/视角/跳跃 → 菜单/商店 → 背包 → 一个普通技能 → 基础攻击/蓄力/连招 → 其余技能`。每迁一个 Action，移除旧路径上**同一 Action** 的绑定，再做 PIE。Controller 当前 UI Action 的 `Triggered` 绑定可能随着按住而重复执行菜单切换；是否真的重复要按资产的 Trigger 配置和 PIE 观察判断。若确实是单次开关，实施时改用适合的单次事件（常见为 `Started`）。[当前位置](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerController.cpp:55)

### 技能输入先写出“事件契约”

| 操作 | 旧行为 | 新 Handler 必须明确的行为 |
| --- | --- | --- |
| 按下 | `AbilityLocalInputPressed(InputID)` | 用正确的 ASC 和 InputID 执行一次按下；不要在每帧触发时重复按下 |
| 松开 | `AbilityLocalInputReleased(InputID)` | 明确发送 Released，还是把该技能设计成“松开取消”；二者不同 |
| 基础攻击 | 按下/松开还发送项目 Gameplay Event | 验证连招事件是否仍需要，是否在客户端和服务端各发生一次 |
| 技能升级 | 旧角色保留升级入口 | 确认升级指令最终到服务器及正确的 ASC；不要因为输入迁移丢掉等级功能 |

当前 [UInputHandler_AbilityInputPlus::ActivateAbility](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Input/Handlers/InputHandler_AbilityInputPlus.cpp:48) 调用 Pressed；其 [CancelAbility](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Input/Handlers/InputHandler_AbilityInputPlus.cpp:64) 调用 `InterruptAbilityByInputID`。Maple 的基类在 `Completed/Canceled` 时调用 `CancelAbility`。[源码](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Input/Handlers/InputHandler_AbilityActivation.cpp:94) **因此它当前不能直接等价替换旧的 Pressed/Released 路径。** 先选一个没有长按/松开语义的普通技能做切片；蓄力、瞄准确认、连招需要分别设计并验收。

背包输入现在由 [ACPlayerCharacter::UseInventoryItem](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerCharacter.cpp:152) 把 float 转为槽位索引。迁移时可以用项目自定义 Handler 传给当前 Pawn 的 `UInventoryComponent`，或定义清晰的 Pawn 接口；不要把背包组件直接搬到 Controller。

### 每个 Input 切片的验收表

| 项目 | 本地单机 PIE | 监听服务器 | 客户端 | 换 Pawn/重生 |
| --- | --- | --- | --- | --- |
| 事件只进一次 | □ | □ | □ | □ |
| 当前 Pawn/ASC 正确 | □ | □ | □ | □ |
| 按下/松开或持续触发正确 | □ | □ | □ | □ |
| 菜单焦点、输入禁用/恢复正确 | □ | □ | □ | □ |

死亡、眩晕、比赛结束当前通过 Pawn/Controller 的 `EnableInput/DisableInput` 阻止输入。[角色代码](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerCharacter.cpp:158) 迁移后需要决定是用 `Manager->DisableInputProcessing()`、GAS 阻挡 Tag，还是分别限制 Gameplay 和 UI；**不要简单把全部输入关掉**，否则死亡界面或菜单也可能无法操作。Maple Manager 的禁用函数会清除缓冲。[源码](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleInput/Source/MapleInput/Private/Components/MapleInputManagerComponent.cpp:710)

## 6. Input 稳定后再处理 Character 继承

上一轮对话的方向是保留 AI/小兵的 MapleGASCharacter 分支，另建 MapleGASPlayerCharacter 玩家分支。本次源码仍支持这个方向，但需要先处理旧 ASC、蓝图父类和项目公共行为。

```text
                    ACharacter
                        │
              AMapleGASCharacter
               ├────────┴───────────────────────┐
               │                                │
          ACCharacter                 AMapleGASPlayerCharacter
          （AI/小兵）                          │
                                  新项目玩家 C++ 基类
                                            │
                                       BP_Player
                                      /         \
                               Crunch BP      Phase BP

          公共的项目规则 → ActorComponent / 接口 / 数据资产
```

Maple 玩家类明确不创建 Pawn ASC，而是从 PlayerState 取 ASC 并调用 `InitAbilityActorInfo(PlayerState, Pawn)`。[AMapleGASPlayerCharacter](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleGAS/Source/MapleGAS/Private/GameFramework/MapleGASPlayerCharacter.cpp:8) 现有 `ACPlayerState` 还不是 Maple PlayerState，因此不能只改玩家 Character 的父类就期待技能自然接通。

| 从当前 `ACCharacter`/`ACPlayerCharacter` 搬运的内容 | 推荐归宿 | 先检查什么 |
| --- | --- | --- |
| 相机臂、相机偏移、背包组件 | 新项目玩家基类或专用组件 | 蓝图子类是否覆盖组件属性、相机位置和背包引用 |
| 头顶血条、死亡/复活表现、Tag 监听 | 共享 ActorComponent，必要时由玩家/AI 分别配置 | 依赖的是 Pawn ASC 还是 PlayerState ASC；Tag 是否与 MapleCombat 一致 |
| 团队、目标捕获接口、AI 感知 | 按玩家/AI 需求分别留在分支或抽接口 | 小兵、技能、Widget 是否强制 Cast 到 `ACCharacter` |
| 旧 `UCAbilitySystemComponent`、`UCAttributeSet` | 迁移到 MapleGAS 方案后再清理 | 只有一个 ASC、所有 GA/属性/UI 都从目标 ASC 读取；蓝图不再引用旧组件 |
| 角色定义的 `TSoftClassPtr<ACCharacter>` | 过渡期新增玩家 Pawn 类字段，验证后替换 | 选角、GameMode 生成、展示模型与旧 UI 数据来源 |

**不要先删除 `ACCharacter` 或直接给现有玩家蓝图换父类。** 当前 [UPA_CharacterDefinition::CharacterClass](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Character/PA_CharacterDefinition.h:35) 限定 `ACCharacter`；[ACPlayerState::GetSelectedPawnClass](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Player/CPlayerState.cpp:43) 又从这里生成 Pawn。应先在调试 GameMode 中跑通一个新玩家分支，再迁移 Crunch 和 Phase 的蓝图，最后退役旧分支。

之后再接 MapleCombat。它的 Combat Manager 会监听 `Combat.State.Dead`、`Combat.State.Staggered` 等 Tag，当前项目 `ACCharacter::BindGASChangedDelegate` 监听项目自己的死亡/眩晕/瞄准/聚焦 Tag。[MapleCombat 监听](/Users/sunthemac/UnrealProjects/MaplePlugins/MapleCombat/Source/MapleCombat/Private/Components/MapleCombatManagerComponent.cpp:500) · [项目监听](/Users/sunthemac/UnrealProjects/MMO_Crunch/Source/MMO_Crunch/Private/Character/CCharacter.cpp:369) 要先列出“哪一个 GameplayEffect 产生哪个 Tag、由谁消费”，避免同时触发两套死亡流程。

## 7. 正式实施前的编辑器检查与记录

请在 UE 编辑器或可用时的 VibeUE 中，为下面每项记下**实际值**，不要依据 `.uasset` 文件名推测：

1. `BP_Player`、`BP_AI`、Crunch/Phase 玩家蓝图的 **Parent Class** 与组件树：ASC 有几个？Input Manager 有几个？
2. `IS_Crunch` 的 IMC、Priority、每个 Handler 的 `InputActions`、`TriggerEvents`、`InputID`；Action 类型是 Bool、Axis1D 还是 Axis2D？
3. 当前地图/GameMode 实际使用的 PlayerController、PlayerState、Default Pawn、选角后 Pawn Class。
4. `DA_CrunchGAS` 的 Owner/Avatar 用途、AttributeSet、GA、InputID 和是否在蓝图中赋值。
5. Gameplay、UI、背包三个 IMC 的 Action 是否重叠；切换商店与游戏菜单时焦点和输入模式如何变化。
6. 记录当前“能工作”的功能：移动、视角、跳跃、基础攻击、四个技能、技能升级、背包、商店、死亡/眩晕/重生、选角与跨关卡。

这些检查结果会把本文的**建议**变成可执行的具体修改列表。没有这份资产记录，C++ 源码足以指出依赖关系，但不足以确认当前蓝图运行路线。

## 8. 怎样判断重构成功与是否优化了项目

每个阶段都保留一个可回退的 Git 检查点，先编译，再在独立调试地图测试，最后再进正式关卡。对于每段新改或删除的项目代码，按本项目约定添加 `// Codex: ...` 注释，说明改动目的，便于 review；蓝图改动在提交说明中列资产与节点变化。本文没有修改源代码。

“优化”不要只看类变短。迁移前后用**同一地图、同一角色、同一客户端数**记录：Game Thread 帧时间、输入到动作的可见响应、ASC/Handler/IMC 数量、网络技能激活次数、换 Pawn 后残留绑定。MapleInput 增加了一层分发和可选缓冲，结构更清晰并不自动意味着 CPU 更快。更可靠的成功标准是：行为一致、重复绑定/重复 ASC 消失、角色职责更清楚、后续增加一个 Action 所需修改更少。

### 第一个实际任务

只做第 7 节的资产记录，再完成第 4 节的 **一个 Debug Action 实验**。回报结果时请提供：Controller/Pawn 蓝图父类、Manager 所在位置、`IS_Crunch` 的一条 Handler 配置，以及本地 PIE 与一个客户端的事件日志。依据这些结果再确定第一处项目代码如何改；此时仍不需要动 Character 继承。
