# Crunch 重构 Steps

> 本步骤按 Downloads 中的《Crunch 重构方案.md》整理。目标是把 Crunch 接入 MapleInput、MapleGAS、MapleCombat、MapleInventory；按阶段推进，每次只处理一条链路。

## 执行约定

- 先在调试角色上迁移，验收后再切正式角色。
- 每个阶段通过后留一个可回退点，再进入下一阶段。
- 当前阶段只改当前链路，不同时启用新旧两套输入、ASC、伤害或死亡处理。
- Maple 插件源码先保持不动。第 3 阶段的复活接口需求到时单独评估。
- 本文件是操作路线，不代表任何资产或代码已经完成迁移。

## 总路线

| 阶段 | 目标 | 通过条件 |
|---|---|---|
| 0. 建立基线 | 确定实际角色、当前 ASC 与已有行为；准备调试角色和回退点 | 能指出当前运行用的角色蓝图、ASC、初始化入口和关键数值 |
| 1. 统一 ASC 与属性 | 使用唯一 Maple ASC；迁移属性集、基础值、初始 GE 和 GA 授予 | 默认 GA 只授予一次；客户端能读到正确属性；基线数值一致 |
| 2. 接入 MapleInput | 先移动和 UI，再普攻／上勾拳，最后技能槽、确认取消和物品输入 | 一次按键只触发一次；选点、连招、按键冲突在双端正确 |
| 3. 迁移伤害与战斗组件 | 接入 PendingDamage、CombatManager 和公共战斗结算，再处理死亡／复活 | 服务器伤害、眩晕、击退、奖励和重复复活通过验收 |
| 4. 接入库存 | 迁移库存、快捷槽、物品使用和商店操作 | 物品只消费一次；快捷槽不会指向错误物品；客户端不能越权 |
| 5. 接入控制权恢复 | 验证 AI 接管、断线和重连时 Pawn、ASC、库存连续 | 控制权切换后没有旧 Controller 引用或重复初始化 |
| 6. 清理旧路径 | 删除已被替代的旧输入、ASC、死亡和库存逻辑 | 正式角色只保留一套处理链，完整验收通过 |

## 阶段 0：建立基线

### 0.0 分清英雄选择数据与 ASC 所有权

**把 Lobby 的英雄选择保存在 PlayerState，符合长方案；ASC 放在哪里是另一项设计。**

长方案写明 GameMode 完成选角后按英雄定义生成 `BP_Crunch`，把玩家身份／队伍与角色关联到 PlayerState；同一段明确说 PlayerState 不提供 ASC。后文也写明 `ACPlayerState` 保存英雄选择和会话信息，但不管理 ASC。因此，把 Lobby 的英雄选择保存在 PlayerState，与方案一致。

实现上可以让 GameLevel 的 GameMode 读取 PlayerState 中的英雄标识，解析出角色定义、Pawn 类和对应的 GA 配置；生成的 Pawn 使用自己的 Maple ASC，服务器从该角色配置初始化 GA。**原文没有写出选择字段、服务器校验和“英雄选择 → 角色定义／GA 配置”的具体映射**；若支持多个英雄，需要把这段补充进方案。PlayerState 保存英雄 ID 或英雄定义引用，不等于它持有 ASC。

截图里的 `BP_MapleGasPlayerState` 还添加了 `MapleGASAbilitySystemComponent`，并设置 `DA_CrunchGAS`。这一步表达的是“PlayerState 持有 ASC 并配置 GA”，与长方案选择的 Pawn ASC 不同。截图不能单独证明它就是当前 GameMode 正在使用的 PlayerState 类，需核实实际运行配置。

源码继承链是 `ACPlayerCharacter → ACCharacter → AMapleGASCharacter`，但它本身不能说明 Lobby 选择怎样传到 GameLevel，也不能说明运行时 ASC 属于谁。Maple 的 `AMapleGASPlayerCharacter` 是从 PlayerState 获取 ASC 的专用类；采用该类才是 PlayerState ASC 路线。

当前 C++ 还要核对旧 ASC：`ACCharacter` 继承 `AMapleGASCharacter`，同时 `CCharacter.cpp` 创建 `UCAbilitySystemComponent` 并让 `GetAbilitySystemComponent()` 返回它。先核实 GameMode 使用的 Pawn／PlayerState 类和组件树，再决定 ASC 迁移路线；不据类名推断运行时只用了其中一个。

### 0.1 先看实际运行的角色

在编辑器里确认当前地图使用的角色蓝图。方案用 `BP_Crunch` 表示目标角色；工程里实际资产可能叫 `BP_Player` 或其他名字，以当前 GameMode / Pawn 配置为准。

### 0.2 确保 C++ 基线能编译

当前 `GA_Combo.h` 的父类是项目的 `UCGameplayAbility`，但头文件还保留着 `CombatAbility_Attack.h` 和 `CombatAbility_MeleeAttack.h` 两个 MapleCombat include。启用的 MapleCombat 中没有 `CombatAbility_MeleeAttack.h`，而这个头文件也没有使用 `UCombatAbility_Attack` 类型。

先只清理这两个无用的 MapleCombat include，保留 `CGameplayAbility.h` 和 `UCGameplayAbility` 父类；编译确认基线后再继续。此步骤是清理上次父类尝试留下的引用，不是开始迁移 GA_Combo 行为。

### 0.3 记录当前行为

先记录，不修改：

- 当前角色蓝图和 ASC 组件名称、类名。
- 角色属性、GA、GE 分别由哪里初始化和授予。
- 当前普攻、技能、选点、死亡／复活是否能正常运行。
- 当前用于测试的关卡、网络模式和测试目标。

### 0.4 准备回退点

- 复制一个调试用 Crunch 角色蓝图，正式角色暂不改。
- 保存当前资产副本或建立版本控制检查点。
- 记录一份基础属性和技能结果，后续用作对照。

**进入阶段 1 的条件：**C++ 基线可编译；知道实际角色蓝图是哪一个，并完成组件与初始化入口盘点。

## 阶段 1：统一 ASC 与属性

### 1.1 找出当前所有 ASC

源码初步核对发现：`ACCharacter` 继承 MapleGAS 的 `AMapleGASCharacter`；MapleGAS 基类创建 `UMapleGASAbilitySystemComponent`。项目子类又创建 `UCAbilitySystemComponent`，并让 `GetAbilitySystemComponent()` 返回项目 ASC。

因此要先确认蓝图组件树和运行时初始化到底用了哪一个。**此时不要删除任何组件。**

### 1.2 盘点旧 ASC 的职责

检查项目 `UCAbilitySystemComponent` 及其调用方，把职责逐项列出来：

- 基础属性初始化。
- 初始 GameplayEffect。
- 默认 GA 与被动 GA 授予。
- 技能升级、经验和等级更新。
- 死亡标签、满血／空蓝等属性监听。

再把它们对应到 MapleGAS 的 `UMapleGASDataAsset`、AttributeSet 和项目成长组件。迁移完成前，旧逻辑仍是对照依据。

### 1.3 只在调试角色上切换

- 目标是一个 ASC：GA、UI、输入、伤害、库存都必须查询到同一个实例。
- 配置 `DA_CrunchGAS`、AttributeSet 和 `FAttributeMetaData` 数据。
- 将默认 GE 与 GA 授予迁到 MapleGAS 配置，检查不会重复授予。
- 先保留正式角色和旧实现，避免一次切换影响整个工程。

**阶段验收：**组件树和接口指向同一个 ASC；默认 Spec 数量正确；属性初值、恢复效果和客户端复制与基线一致。

## 阶段 2：接入 MapleInput

按下面顺序逐类迁移。每完成一类，就检查旧绑定是否仍会重复触发。

1. **移动与视角**：使用 MapleInput Manager 和对应内置 Handler。
2. **UI 输入**：单独装载 `IS_PlayerUI`，确认菜单打开时仍能关闭菜单。
3. **普攻与上勾拳**：使用内置 `UInputHandler_CombatCombo`。按长方案保留 Montage Section；用 ComboWindow 标签和 `ability.basicAttack.pressed` 推进，不迁到 ComboManager／StateTree。
4. **技能槽**：按 GA 的 `Ability.input.*` 标签激活对应 GA。
5. **确认／取消和快捷槽**：先迁内置确认取消；物品槽使用项目侧薄 Handler 解析槽位。
6. **装载与卸载**：处理瞄准、死亡、恢复、切换 Pawn、菜单占用时的 Setup 生命周期。

**阶段验收：**一次输入只路由一次；窗口外按键不接段；确认键不同时触发普攻；松开、失焦和切 Pawn 不留输入状态。

## 阶段 3：伤害与战斗组件

1. 先在测试目标上迁移公共伤害结算，确认攻击者和目标都使用正确 ASC。
2. 接入 `UMapleCombatAttributeSet.PendingDamage` 与项目伤害计算规则；对比原有伤害结果。
3. 接入项目扩展 `UCrunchCombatManagerComponent` 和 `DA_CrunchDamage`。
4. 再迁移死亡判定、死亡表现和复活流程，覆盖取消、超时和重复触发。

方案把 MapleCombat 的可恢复死亡入口列为必需扩展。到这一步先检查现有 API 能否满足；如确需改插件源码，单独审查最小改动和回归范围，不与前三阶段混做。

**阶段验收：**服务器只结算一次伤害和奖励；重复死亡／复活不销毁 Pawn、不叠加 Timer，也不留下冷却、窗口或移动状态错误。

## 阶段 4：库存与物品

1. 在调试角色上创建 MapleInventory Manager 和默认容器。
2. 建立快捷槽到物品稳定 ID 的映射，不依赖容器排序位置。
3. 迁移使用物品、消费效果、商店购买／出售和能力授予。
4. 验证移动、出售、用尽物品后快捷槽会更新且不会误用其他物品。

**阶段验收：**消耗和交易由服务器校验且只执行一次；装备效果和授予能力各有唯一管理方。

## 阶段 5：AI、断线与重连

- AI 接管同一个 Pawn 和 ASC，不重复授予 GA 或初始化库存。
- 玩家重连后重新绑定当前 Pawn，先读取属性／库存快照，再接增量更新。
- 旧 Controller 失去操作权和库存代理权限。

**阶段验收：**AI 与新玩家交替接管后，Pawn、ASC、等级、冷却和库存保持连续。

## 阶段 6：清理与完整验收

- 只删除已经由新路径完整替代的旧实现。
- 覆盖 Dedicated Server＋两客户端；单机 PIE 不作为网络验收结论。
- 按原方案检查伤害互通、死亡复活、快捷槽、断线重连和对象回收。

## 现在从这里开始

当前只做阶段 0 的组件盘点：

1. 先在 `GA_Combo.h` 移除两个未使用的 MapleCombat include，保留项目父类；编译确认当前基线。
2. 打开当前真正运行的 Crunch 角色蓝图。
3. 在 **Components** 面板记录所有 Ability System 相关组件的名称与类名。
4. 暂时不要删除组件或改初始化逻辑，把编译结果和组件列表截图或名称发回本对话。

我会根据这份清单带你完成阶段 1.2 的职责对照，再决定调试角色上应迁移的第一项。蓝图资产需要在 UE 编辑器中确认；当前会话没有可调用的 VibeUE 工具。
