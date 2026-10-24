# Crunch 重构方案

> 目标：让 Crunch 优先复用 MapleCombat 的包装 GA；保留确属 Crunch 的行为。方案只改项目侧 GA / 配置，不改 Maple 插件源码。

## 资料边界

- `Crunch.zip/Crunch.md`：当前技能行为基线。
- `Reactor advice.md`：架构建议，不视为当前工程事实。
- 以下映射按当前安装的 MapleCombat 类与项目 C++ 静态源码判断；蓝图父类和资产字段仍需在编辑器核对。当前会话没有 VibeUE，未把 `.uasset` 名称当成已验证配置。

## GA 替换建议

|Crunch 技能|MapleCombat 目标|建议|保留 / 迁移重点|
|---|---|---|---|
|`GA_GroundBlast`|`UCombatAbility_Cast`|**先做试点**|插件已支持确认式选点。Crunch 的选点半径/距离、确认后扣费、伤害击退、Cue 和施法 Montage 逐项映射；现有 `ATargetActor_GroundPick` 是否能直接配置待核。|
|`GA_UpperCut`|`UCombatAbility_Attack`|**拆成包装 GA + Crunch 扩展**|复用攻击事件、近战扫描和命中效果；上挑击飞、空中接段、Section 伤害表仍由 Crunch 扩展处理。命中扫描和伤害管线要迁到 MapleCombat 后，才算真正替换。|
|`GA_Combo`|`UCombatAbility_Combo`|**作为第二阶段目标，不直接换父类**|插件 Combo 由 Combo Tree / Combo Manager 编排多个子 GA；Crunch 当前是单 GA 播放四段 Montage、事件切 Section、按 Section 选 GE。需拆成子攻击并重做状态树，行为核对后再切。|
|`GA_Dash`|暂留 Crunch GA|**不换成 `UCombatAbility_Evade`**|Dash 是前冲、范围命中、伤害和击退；Evade 负责闪避状态与无敌效果，语义不匹配。只有把 Dash 设计改成防御闪避时才考虑 Evade。|
|`GA_Tornado`|暂留 Crunch GA|**暂不迁**|现有资料记载 Montage / 命中 GE 尚未配置。先定清楚持续命中行为，再判断是否能用 `UCombatAbility_Attack` 承接。|

> **连招路线说明**：本表的 `UCombatAbility_Combo` 是后续 StateTree 路线；ZIP 详细方案则保留 Crunch `UGA_Combo`，先用 MapleInput `UInputHandler_CombatCombo` 接管输入。两者不是同一套配置，见[流程梳理](BasicAttack与GA_Combo梳理.md)。

> **当前代码状态**：之前试做的 Combo / UpperCut C++ 接入已撤回，cpp 恢复为项目原有实现。`GA_Combo.h` 保留当前手动尝试的 `UCombatAbility_MeleeAttack` 父类声明；启用的 MapleCombat 6.6.17 没有这个类，继续修改前要先确认真实父类并让头文件与 cpp 对齐。Combo Manager / StateTree、Handler 资产和窗口 GE 仍是未实施目标。

**关键点**：复用包装 GA 不是只改 C++ 父类。原 GA 的 `ActivateAbility`、扫描、扣费、伤害与结束流程若与插件流程并行，会造成重复激活或重复结算；应让插件负责通用流程，Crunch 子类只补专属逻辑。

## 三个插件怎么用

- **MapleInput**：优先用内置 Ability Input Handler 替换项目自定义 InputID 分发；普攻按下/松开、Confirm/Cancel 事件逐项保留，避免新旧输入同时绑定。
- **MapleGAS**：用 Setup Data Asset 管理默认技能、初始效果和属性集；先统一角色实际使用的 ASC，再移除重复授予/初始化入口。
- **MapleCombat**：优先落地 Cast，再评估 Attack 和 Combo。先核对 Crunch 的 `UCAttributeSet`、GE 与 Combat 伤害组件能否共用；不兼容时保留项目 GE，由项目侧薄扩展衔接，不改插件源码。

## 迁移顺序

1. **确认 ASC 唯一性**：当前 `ACCharacter` 继承 MapleGAS 角色，又创建项目 `UCAbilitySystemComponent`。先确认 GA、输入、GE 和 MapleCombat 组件读取同一个 ASC。
2. **GroundBlast 试点**：以 `UCombatAbility_Cast` 承接确认式选点，只迁这一个技能；先核对确认/取消、扣费时机、TargetData、伤害与 Cue。
3. **UpperCut 攻击流程**：接入 `UCombatAbility_Attack` 的扫描和命中流程，Crunch 保留击飞与空中连段规则。
4. **Combo 状态树**：把四段攻击拆为子 GA，配置 Combo Tree，再验证输入窗口、段间转换和每段伤害。
5. **Dash / Tornado**：仅在行为与包装类职责匹配后迁移；不为追求“全换插件”改变技能设计。

## 完成标准

- 一次输入只激活一次 GA；确认、取消、按下/松开语义不变。
- Crunch 特有的击飞、空中接段、伤害段表、Dash 位移与取消清理仍正确。
- 伤害只结算一次；GA / GE / 属性集没有重复授予或重复初始化。
- Maple 插件源码无改动；未来如需项目 C++ 变更，按约定用 `// Codex:` 标记。
