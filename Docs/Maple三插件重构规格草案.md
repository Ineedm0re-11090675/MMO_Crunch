# MMO_Crunch：Maple 三插件重构提纲

> 按你的草稿扩写。下文“当前”指 C++ 可见逻辑；“建议”指待实施配置。蓝图和 Data Asset 字段仍需在 UE/VibeUE 中核对。本次只改文档。

# data

## Input Setup Data Asset（MapleInput）

- 玩家通用：Move、Look、Jump、Shop、Menu、背包快捷键。
- Crunch：LMB → BasicAttack/Combo；Slot1 → UpperCut；其他槽位 → Dash、GroundBlast、Tornado（具体键位待定）。
- Phase：LMB/RMB → Shoot/瞄准；技能槽 → Blink、Freeze、Lazer、Blackhole；Confirm/Cancel → 当前正在选目标的 GA。
- 每个 Input Action 记录：TriggerEvent（Started/Triggered/Completed/Cancelled）、Handler、发送的 Tag、按住和松开行为。避免按住时每帧重复激活。
- IS_Player 资产已存在，实际 IMC 和 Handler 待核对。AI 不默认用玩家 Input Setup。

## GAS Setup Data Asset（MapleGAS）

- Crunch、Phase、Minion 各有一份 UMapleGASDataAsset。
- DefaultAttributeSets：初始 AttributeSet、初始化表、重生时 Reset/Keep。
- DefaultGameplayEffects：出生时的基础属性和状态 GE。
- DefaultGameplayAbilities：出生时授予的 GA；Crunch 包括 Combo、UpperCut、Dash、GroundBlast、Tornado；Phase 包括 Shoot、Blink、Freeze、Lazer、Blackhole；Minion 包括 Combo、受击/死亡能力。这是迁移清单，实际资产字段待核对。
- InitialGameplayTags：出生时确实存在的状态；技能身份 Tag 不应当成常驻状态。
- 装备授予和升级解锁另行管理，保存 SpecHandle；卸装和重生时明确撤销或保留。
- 该资产的 Input 是整数 Spec.InputID。MapleInput 按 Tag 激活时，输入槽 Tag 需要单独配置。

## Combat Data Asset（MapleCombat）

- CombatManagerComponent.DamageData → UMapleCombatDamageDataAsset。草稿中的 DamageHandleDataAsset 不是当前插件类名。
- DamageOutcomes：服务端处理死亡、击倒、硬直等伤害后果。
- DamageCosmetics / GameplayCue：特效、音效、镜头、伤害数字；不负责扣血或发奖励。
- DefenseData：格挡、护甲、减伤。先映射旧 Armor/伤害 GE 与插件属性。
- 死亡配置：布娃娃、死亡存活时间、流程超时；小兵还要确定销毁还是回收到对象池。

# GameplayAbility

## 每个 GA 都记录

- 身份：唯一 Ability Tag（如 ability.uppercut）与输入槽 Tag（如建议新增的 ability.input.slot1）。
- 来源：初始授予、升级解锁、装备临时授予。
- 条件：死亡/眩晕、资源、冷却、目标、距离；在 Activation Tags、Cost/Cooldown、CanActivate 和目标校验中落实。
- 冲突：BlockAbilitiesWithTag、CancelAbilitiesWithTag、ActivationOwnedTags、ActivationBlockedTags 分别配置。
- 行为：激活 → 选目标/播 Montage → 发送或等待 Gameplay Event → 服务端应用 GE → 执行 GC → End/Cancel 清理。
- 清理：Timer、持续 GE、TargetActor、委托、缓存目标、Montage Section 和输入状态。

## Ability Tag / Input Tag

- 唯一身份：ability.basicAttack、ability.uppercut 等。
- 输入分类：ability.input.LMB、ability.input.RMB、ability.input.slot1 是建议的新约定，目前不是项目中已确认存在的 Tag。
- 可换技能的槽位不要写死在 GA 身份标签；输入、技能 UI、升级共用“槽位 → 当前 SpecHandle/技能”映射。
- 当前项目还有 AbilityEnum.BasicAttack 等旧命名。先列生产者和消费者，再迁移/清理。

## Instancing Policy

- PerActor：推荐给当前多数 GA。适合等待事件、Montage、Timer 和成员状态；每次重新激活前重置上次状态。
- PerExecution：适合低频、每次执行都需要全新状态的技能；每次新建实例，是否允许多个实例同时运行仍要单独决定，不能用主实例保存“当前运行状态”。
- 如果不允许同技能并发，运行中的 GA 可阻挡匹配自身唯一 Ability Tag 的再次激活；仍要检查输入重复触发与 CanActivate。
- 当前 MapleInput Tag Handler 在取不到 PrimaryInstance 时回落到 CDO，并用 IsActive 判断是否运行。PerExecution 的“活动中再次按键发送事件/切换取消”需要单独验证或调整 Handler，不能只配 Block Tag。
- NonInstanced 不适合这些含蓝图逻辑、异步任务和可变成员的技能。

# General

## BasicAttack / Combo

- 当前类：UGA_Combo；当前 Tag：ability.basicAttack，另有旧 AbilityEnum.BasicAttack。
- 输入：首次按下激活；活动中再次按下推进连招；松开是否结束须明确。
- 当前行为：WaitInputPress 等下一次按键；ability.combo.change.* 选下一 Montage Section；ability.combo.damage 携带目标；Section → 对应伤害 GE。
- Block Tag：当前代码阻挡 ability.basicAttack；还要列出与 UpperCut、Dash 等技能的打断关系。
- MapleCombat 的 UCombatAbility_Combo + UMapleCombatComboManagerComponent + StateTree 是另一套连招方案。先记录当前节奏，再决定是否迁移；Montage Section 不能直接当 StateTree 状态。

## 受击、死亡、击飞

- 当前 UGAP_Dead 根据旧死亡 Tag 触发，还分配经验/金币；UGAP_Launched 响应击飞事件。
- 新方案由 MapleCombat 伤害结果驱动 HitReaction、Death、KnockDown 等 GA。
- 旧 ACCharacter::StartDeathSequence、旧 Death GA 和新 Damage Outcome 只能保留一个权威死亡/奖励入口。
- 复活或对象池重新激活时恢复输入，清理旧目标、Tag、GE 和委托。

# Crunch

## UpperCut

- 当前类/资产：UUpperCut / GA_UpperCut_BP；建议 Ability Tag：ability.uppercut；建议输入：Slot1。
- 当前 Block：阻挡 ability.basicAttack。若 UpperCut 不允许自身重入，还需配置自己的阻挡规则并验证。
- 行为：播 UpperCut Montage → 等 ability.uppercut.launch → 推起自身与目标 → 应用 LaunchDamageEffect → 可接收连招变化与再次按下。
- Section → GE：当前代码使用 Montage Section 查询 DamageMap，不是 Chooser 资产。若改用 Chooser，先决定它只选动画，还是也选伤害 GE。
- 验收：命中/空挥、连续按键、中断、客户端预测、服务端推力与伤害。

## Combo

- 资产：GA_Combo_Crunch_BP、AM_Combo_Crunch、四段 GE_Crunch_Combo_Damage_*。
- 每一段记录输入窗口、Section、命中 Notify、目标组、GE，以及能否被 UpperCut/Dash 打断。

## Dash

- 当前 UGA_Dash 等 ability.dash.start，添加 Dash GE、前推移动，用 TargetActor 检测目标。
- 结束/取消时移除 Dash GE、Timer 和目标检测；服务端确认伤害。

## GroundBlast

- 当前 UGA_GroundBlast 选择地面范围，Confirm 后 Commit，应用伤害 GE 和爆炸/镜头 GC。
- Cancel 或无效位置不扣费；记录范围、射程、敌我过滤、重复命中规则。

## Tornado

- 当前 UGA_Tornado 播 Montage、等伤害事件、应用 GE，并可 Cancel/延时结束。
- 记录事件发送者、命中频率、同目标重复受伤规则及中断清理。

# Phase

## Shoot

- 当前 UGA_Shoot 由基础攻击 Pressed/Released 控制 Montage；ability.shoot.* 事件触发投射物；持续检查锁定目标并通知 UI。
- 分清 LMB/RMB、按住射击、松开停止、瞄准与目标锁定。投射物与命中 GE 由服务端确认；结束清理目标监听和准星。

## Blink

- 当前 UGA_Blink 选地面点 → Confirm 后 Commit → 等 ability.blink.teleport 动画事件 → 传送/应用 GE。
- 服务端校验距离与落点；Cancel 不扣费；结束清理目标预览。

## Freeze

- 当前 UGA_Freeze 选区域 → Confirm/Commit → 应用冻结 GE 与 GC。
- 区分控制、伤害和表现；定义重复冻结与敌我过滤。

## Lazer

- 当前 UGA_Lazer 播 Montage → 等射击事件 → 施加持续消耗 GE → 线形 TargetActor 周期命中；资源耗尽或 Cancel 结束。
- 分清按住、松开、取消；结束移除持续 GE，限制同目标命中频率。

## Blackhole

- 当前 UGA_Blackhole 选放置点 → Commit → 吸附/保持 → FinalBlow GE 与击退。
- 记录“放置中、吸附中、终结、取消”四阶段的输入、Montage、TargetActor、GE；核对取消回调、TargetData 索引和清理路径。

# Minion

## Combo

- 资产：GA_Combo_Minion_BP、AM_Combo_Minion、四段伤害 GE。
- AIController/行为树决定何时激活；不依赖本地玩家 Input Action。确认攻击间隔、仇恨目标、命中与死亡打断。

## 死亡和对象池

- Minion GAS Setup 配初始属性/GE/GA；死亡只结算一次奖励。
- 回收时清目标、Tag、GE、能力任务和委托；再次激活时重新初始化 ASC/Avatar 与战斗状态。

# 层级

## BP-PlayerController extends PlayerController

- Components：一个 MapleInputManagerComponent，加载玩家通用与当前 Pawn 的 Input Setup。
- 负责 UI、菜单、商店、Possess/换 Pawn；不保存技能伤害与角色属性。
- 每次输入找到当前 Pawn 的 ASC；迁移旧 UI IMC/绑定时避免重复响应。

## BP-PlayerState extends MapleGASPlayerState（目标）

- Components：玩家 ASC；背包组件是否放这里，由“跨死亡保留物品”的需求决定。
- 保留选角、队伍、等级/经验/金币。若背包迁入，同时调整 RPC Owner、复制、UI 和装备 GA/GE 授予位置。

## BP-Player extends MapleGASPlayerCharacter（目标）

- Components：MapleCombatManagerComponent、相机/移动表现。ASC 来自 PlayerState，Pawn 是 Avatar。
- Crunch/Phase 子 BP 分别配置 GAS Setup、Input Setup、Damage/Defense Data。
- 当前 ACPlayerCharacter 仍继承 ACCharacter；先处理选角资产的 Pawn 类型与旧 ASC 引用，再换 BP 父类。

## BP-AIController extends AIController

- Components：AIPerception、Brain/Blackboard；负责感知、选目标、行为树/StateTree 决策。
- 不默认挂 MapleInput Manager。AI 技能由服务端行为逻辑调用 ASC。
- BP-AIController 是 BP-AI 的 AI Controller Class 配置，不是 BP-AI 的子组件。

## BP-AI extends MapleGASCharacter（目标）

- Components：Pawn ASC、MapleCombatManagerComponent；只有采用插件 StateTree Combo 时才加 Combo Manager。
- 配置：Minion/AI GAS Setup、Damage/Defense Data、团队、感知源、死亡/对象池策略。
- 背包只在 AI 确有装备或掉落需求时添加。

## BP-GameState / BP-GameMode

- GameState：复制选角、队伍、比赛阶段和胜负。
- GameMode：服务端决定 Pawn 类、出生点、小兵/核心规则与比赛结束。
- 玩家基类迁移后同步调整 UPA_CharacterDefinition 与 ACPlayerState::GetSelectedPawnClass() 的类约束。

# 实施顺序

1. 用 UE/VibeUE 核对 BP 父类、组件、Data Asset 字段，以及每个 GA 的 Tag、输入、GE、GC、Montage/Notify。
2. 跑通一个“输入 → 正确 ASC → GA”的小切片，再分别完成玩家/AI 单 ASC。
3. 逐项迁 GA；UI、升级和背包同时迁到相同的 Spec/ASC 契约；最后决定 MapleCombat Combo 的采用方式。
4. 每项在单机、监听服务器、远端客户端、死亡/换 Pawn 下验证。实施代码变更时按约定加 Codex 修改注释。
