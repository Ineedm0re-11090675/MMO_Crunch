# BasicAttack 与 GA_Combo 梳理

> 本文解释 `Crunch 重构方案.zip` 中的 BasicAttack / GA_Combo 方案。方案文字不代表代码或蓝图已经完成。

## 先记住结论

- `BasicAttack` 是玩家的普攻输入，不是 MapleCombat 的某个 GA 类。
- ZIP 方案保留 Crunch 的 `UGA_Combo` 和 Montage Section，使用 MapleInput 的 `UInputHandler_CombatCombo` 分流左键。
- 这不是迁移到 MapleCombat 的 `UCombatAbility_Combo` / StateTree。

## 四种容易混淆的标签

| 名称 | 用途 |
|---|---|
| `ability.basicAttack` | 当前普攻 GA 的身份标签，也用于能力阻挡。 |
| `Ability.input.basicAttack` | **方案中建议使用的输入槽标签**；Handler 无窗口时用它查找并激活普攻 GA。当前源码和标签配置都还没有接入它。 |
| `Combat.State.ComboWindow` | 临时状态标签。`GE_ComboWindow` 授予到角色 ASC，表示此刻可以接段。 |
| `ability.basicAttack.pressed` | 有接段窗口时发送的 Gameplay Event；通知当前连招 GA 推进，不携带命中目标。 |

`ability.combo.change.*` 和 `ability.combo.damage` 是动画/命中事件，不是输入标签。

## 一次地面普攻是怎样运行的

```text
左键 Triggered
  └─ UInputHandler_CombatCombo 检查角色 ASC
       ├─ 没有 Combat.State.ComboWindow
       │    └─ TryActivateAbilitiesByTag(Ability.input.basicAttack)
       │         └─ 激活 GA_Combo
       └─ 有 Combat.State.ComboWindow
            └─ 发送 ability.basicAttack.pressed
                 └─ 当前等待事件的 GA 接收并推进连招
```

ZIP 给 `UInputHandler_CombatCombo` 的配置可以这样读：

| 字段 | ZIP 中的值 | 作用 |
|---|---|---|
| `InputActions` | `IA_BasicAttack` | 监听普攻输入动作。 |
| `TriggerEvents` | `Triggered` | 按键触发时执行下面的分流。 |
| `ActivationTags` | `Ability.input.basicAttack` | 没有接段窗口时，用这个标签查找并启动 GA。 |
| `ComboStateTag` | `Combat.State.ComboWindow` | 判断 ASC 当前是否有可接段窗口。 |
| `AdvancementTag` | `ability.basicAttack.pressed` | 有窗口时发送这个 Gameplay Event，让当前 GA 接段。 |

这是 ZIP 的配置提案，不代表当前 Handler 资产已经这样设置。落地时，`GA_Combo` 也需要有对应的 `Ability.input.basicAttack` Ability Tag。

1. **开始普攻**：Handler 用 `ActivationTags = Ability.input.basicAttack` 激活带有该标签的 `GA_Combo`。
2. **动画打开接段窗口**：Montage Notify 发送 `ability.combo.change.combo02`。GA 记录 `NextComboName = combo02`，并应用 `GE_ComboWindow`。
3. **玩家在窗口内再按左键**：Handler 看到 `Combat.State.ComboWindow`，发送 `ability.basicAttack.pressed`。GA 才将当前 Montage Section 接到 `combo02`，然后移除本次窗口效果。
4. **命中结算**：命中 Notify 发送 `ability.combo.damage`，Payload 的 `TargetData` 带命中目标。服务器按当前 Section 从 `DamageEffectsMap` 选择伤害 GE。
5. **结束或中断**：`ability.combo.change.end` 清空下一段；窗口被消费、Montage 结束或 GA 被取消时，也要移除自己创建的窗口效果。

注意：`ability.combo.change.combo02` **不是立刻切段**。它是先记录“下一段可以接到哪里”；玩家在有效窗口内按键后，才真正设置 Montage 的下一段。

## 上勾拳为什么也听同一个按键事件

上勾拳通过自己的输入启动。命中并进入空中连招窗口后，同一个 `ability.basicAttack.pressed` 事件可以推进上勾拳的 Montage。它不应带 `Ability.input.basicAttack`，否则无窗口时也可能被普攻 Handler 当作普攻启动目标。

## 当前工作区改动与 ZIP 目标

| 当前工作区 | ZIP 目标 |
|---|---|
| `GA_Combo.cpp` 和 `UpperCut.cpp` 已恢复为项目原有实现。`GA_Combo.h` 保留当前手动尝试的 `UCombatAbility_MeleeAttack` 父类声明；启用的 MapleCombat 6.6.17 没有这个类，需先确认父类名并让头文件与 cpp 一致。 | 左键交给 `UInputHandler_CombatCombo`，按窗口状态选择激活 GA 或发送接段事件。 |
| 当前 `GA_Combo` 仍通过 `WaitInputPress` 等待输入；`GA_Combo` / `UpperCut` 都没有 `ComboWindowEffect` 属性或窗口 GE 的申请、消费与清理逻辑。 | `GA_Combo` 与 `GA_UpperCut` 都配置 `GE_ComboWindow`，收到有效输入后各自消费并移除窗口。 |
| 项目标签配置里没有 `Ability.input.basicAttack`；MapleInput Handler 和蓝图是否已配置，尚未核对。 | UpperCut 不加入 `Ability.input.basicAttack`，但在有效窗口内接收相同的 `ability.basicAttack.pressed` 事件。 |
| 角色当前输入代码还保留项目原有分发流程；ZIP 提议的 MapleInput Handler 路由尚未接入核验。 | 迁移输入绑定时收敛重复按下路径，同时保留射击所需的 Released 语义。 |

当前没有已接入的 `ComboWindowEffect` C++ 属性或 `GE_ComboWindow`。下方编辑器步骤是待实施的方案，不是已经做完的配置。

## 编辑器侧剩余步骤

1. 创建 `GE_ComboWindow`：Duration 设为 Infinite，Granted Tags 添加 `Combat.State.ComboWindow`。
2. 在 `GA_Combo_Crunch_BP` 和 UpperCut 蓝图中设置各自的 `ComboWindowEffect`。
3. 配置 `UInputHandler_CombatCombo`：`IA_BasicAttack`、`Triggered`、`Ability.input.basicAttack`、`Combat.State.ComboWindow`、`ability.basicAttack.pressed`。
4. 确认 `GA_Combo` 有输入槽标签，UpperCut 没有；核对 Montage Notify 发出的 `ability.combo.change.<Section>` 和 `ability.combo.change.end`。
5. 最后在 PIE 检查首次激活、窗口内接段、窗口外按键、打断清理和 UpperCut 空中接段。

## 实施前必须处理的两个风险

- **窗口要有唯一接收者**：`Combat.State.ComboWindow` 是 ASC 上的共享标签，Handler 只知道“有窗口”，不知道窗口属于哪个 GA。方案落地时需要明确窗口归属和消费规则，避免两个 GA 同时响应；之后再用 PIE 验证预测与服务器同步。
- **不要全局删除 Released 事件**：当前 `GA_Shoot` 用 `ability.basicAttack.released` 停止射击。若移除 Character 的 Released 发送，需先为射击保留独立释放路径或调整其输入设计。

## 两份方案的路线差异

- **ZIP 的详细方案**：保留 Crunch `UGA_Combo`，用 MapleInput Handler 改造输入路由；保留 Montage Section，不迁移 Combo Manager / StateTree。
- **仓库短版方案**：把 `UCombatAbility_Combo` + Combo Manager / StateTree 列为后续目标。这是拆分成多个子 GA 的另一种架构，不是上面 Handler 配置的下一行设置。
- 项目实际启用的 MapleCombat 6.6.17 有 `UCombatAbility_Attack` 和 `UCombatAbility_Combo`，没有 `UCombatAbility_MeleeAttack`。不要按另一份插件副本里的类名配置。

## 目前能确认 / 仍需编辑器核对

- 已按当前 C++ 与插件源码梳理 Handler 分流、MapleCombat `UCombatAbility_Attack` 的动画／命中 API。之前试做的项目侧 C++ 接入现已撤回；没有改 Maple 插件源码。
- 当前会话没有可调用的 VibeUE 接口；尚未核对 Input Setup、蓝图标签、窗口 GE 赋值或 Montage Notify 帧，也没有做编译／PIE 验证。

### 代码参考

- [CPlayerCharacter.cpp](../Source/MMO_Crunch/Private/Player/CPlayerCharacter.cpp)
- [GA_Combo.cpp](../Source/MMO_Crunch/Private/GAS/GA_Combo.cpp)
- [UpperCut.cpp](../Source/MMO_Crunch/Private/GAS/UpperCut.cpp)
- [GA_Shoot.cpp](../Source/MMO_Crunch/Private/GAS/GA_Shoot.cpp)
- [UInputHandler_CombatCombo.cpp](/Users/sunthemac/UnrealProjects/UnrealEngine/Engine/Plugins/Marketplace/MapleInput/Source/MapleInput/Private/Input/Handlers/InputHandler_CombatCombo.cpp)
