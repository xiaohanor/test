# Quick Reference: PlayMontage Feature

## Overview
Added a new `PlayMontage` intent to the LLM-driven AI system, allowing AI characters to play animation montages via natural language commands.

## What Changed

### 1. Data Types (LLMActionTypes.h)
- Added `PlayMontage` to `ELLMIntent` enum
- Added montage fields to `FLLMAction`:
  - `MontageName` (String) - Required
  - `MontageSection` (String) - Optional
  - `MontagePlayRate` (Float) - Default 1.0, range [0.1, 5.0]
  - `bMontageLoop` (Bool) - Default false

### 2. Parser (LLMActionParser.cpp)
- **Parsing**: Reads montage fields from JSON
- **Validation**: 
  - MontageName must not be empty
  - PlayRate must be in [0.1, 5.0]
  - Confidence ≥ 0.5
- **System Prompt**: Updated with PlayMontage examples

### 3. Blackboard Mapper (LLMBlackboardMapper.h/.cpp)
Added 4 new Blackboard keys:
- `MontageName` (String)
- `MontageSection` (String)
- `MontagePlayRate` (Float)
- `MontageLoop` (Bool)

### 4. BT Task (BTTask_PlayMontage.h/.cpp)
New task node that:
- Reads montage parameters from Blackboard
- Logs montage play request
- Displays on-screen debug message
- Returns success (MVP implementation)

## Usage Examples

### JSON Format
```json
{
  "intent": "PlayMontage",
  "montageName": "Wave",
  "confidence": 0.9
}
```

```json
{
  "intent": "PlayMontage",
  "montageName": "SwordAttack",
  "montageSection": "Combo1",
  "montagePlayRate": 1.5,
  "montageLoop": false,
  "confidence": 0.95
}
```

### User Input Examples
- "Wave at the player" → `{"intent":"PlayMontage","montageName":"Wave","confidence":0.9}`
- "Attack with sword" → `{"intent":"PlayMontage","montageName":"SwordAttack","confidence":0.95}`
- "Dance slowly" → `{"intent":"PlayMontage","montageName":"Dance","montagePlayRate":0.5,"confidence":0.85}`

## Blackboard Setup (BB_LLM)

Add these 4 new keys to your Blackboard asset:

| Key Name        | Key Type | Description                          |
|-----------------|----------|--------------------------------------|
| MontageName     | String   | Animation montage name               |
| MontageSection  | String   | Montage section to start from        |
| MontagePlayRate | Float    | Playback rate (default 1.0)          |
| MontageLoop     | Bool     | Whether to loop the montage          |

## Behavior Tree Setup (BT_LLM_MVP)

Add a new branch to your Behavior Tree:

**Branch 4: PlayMontage**
- **Decorator**: Check Intent
  - IntentKey: `Intent`
  - ExpectedIntent: `PlayMontage`
- **Decorator**: Blackboard Based Condition
  - Key: `MontageName`
  - Key Query: Is Set
- **Task**: Play Montage (BTTask_PlayMontage)
  - MontageNameKey: `MontageName`
  - MontageSectionKey: `MontageSection`
  - MontagePlayRateKey: `MontagePlayRate`
  - MontageLoopKey: `MontageLoop`

## Testing

### Positive Test Case
**Input**: "Wave at the player"

**Expected Flow**:
1. LLM generates: `{"intent":"PlayMontage","montageName":"Wave","confidence":0.9}`
2. Parser extracts montage fields
3. Validation passes (MontageName not empty, confidence ≥ 0.5)
4. Blackboard keys set: Intent="PlayMontage", MontageName="Wave", MontagePlayRate=1.0
5. BT PlayMontage branch executes
6. Log: `[BTTask_PlayMontage] CharacterName playing montage: Wave (Section: None, Rate: 1.00, Loop: No)`
7. On-screen message: "CharacterName: Playing montage 'Wave'"

### Negative Test Cases
**Missing MontageName**:
```json
{"intent":"PlayMontage","confidence":0.9}
```
- Validation fails: "PlayMontage requires non-empty montageName"
- Blackboard not updated

**Invalid PlayRate**:
```json
{"intent":"PlayMontage","montageName":"Wave","montagePlayRate":10.0,"confidence":0.9}
```
- Validation fails: "PlayMontage play rate 10.00 out of valid range [0.1, 5.0]"
- Blackboard not updated

## Implementation Notes

### MVP Status
The current implementation:
- ✅ Parses montage parameters from JSON
- ✅ Validates montage requirements
- ✅ Writes to Blackboard
- ✅ Logs montage play request
- ✅ Displays debug message
- ⚠️ Does NOT actually load or play animation assets (TODO for production)

### Production TODO
For full production implementation, update `BTTask_PlayMontage::ExecuteTask()`:
1. Load montage asset by name from content directory
2. Call `AnimInstance->Montage_Play(Montage, PlayRate)`
3. Jump to section if specified: `AnimInstance->Montage_JumpToSection(SectionName)`
4. Set up looping if requested
5. Optionally wait for montage completion before returning success

Example code is included as comments in BTTask_PlayMontage.cpp.

## Files Modified/Added

### New Files
- `Source/testcpp/AI/BTTask_PlayMontage.h`
- `Source/testcpp/AI/BTTask_PlayMontage.cpp`

### Modified Files
- `Source/testcpp/LLM/LLMActionTypes.h`
- `Source/testcpp/LLM/LLMActionParser.cpp`
- `Source/testcpp/LLM/LLMBlackboardMapper.h`
- `Source/testcpp/LLM/LLMBlackboardMapper.cpp`
- `LLM_AI_SETUP.md`
- `IMPLEMENTATION_SUMMARY.md`

## Logging

All montage-related actions are logged with the `[BTTask_PlayMontage]` prefix:
```
[LLMActionParser] Parsed action - Intent: PlayMontage, Confidence: 0.90
[LLMBlackboardMapper] Set Intent: PlayMontage
[LLMBlackboardMapper] Set MontageName: Wave
[LLMBlackboardMapper] Set MontagePlayRate: 1.00
[LLMBlackboardMapper] Set MontageLoop: false
[BTTask_PlayMontage] CharacterName playing montage: Wave (Section: None, Rate: 1.00, Loop: No)
```

## Next Steps

1. **Create/Update Blackboard Asset**: Add 4 new montage keys to BB_LLM
2. **Update Behavior Tree**: Add PlayMontage branch to BT_LLM_MVP
3. **Test**: Use GenerateAction async node with commands like "Wave at the player"
4. **Production**: Implement actual montage loading and playback in BTTask_PlayMontage

For detailed setup instructions, see `LLM_AI_SETUP.md`.
