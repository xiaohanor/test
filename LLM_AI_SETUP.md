# LLM-Driven AI Setup Guide (M1-M3 MVP)

This guide explains how to set up the MVP system for controlling Unreal Engine characters with LLM via JSON and Behavior Tree.

## Overview

The system converts natural language input into structured JSON actions, which are then parsed, validated, and executed through a Behavior Tree. The implementation follows three milestones:

- **M1**: Input → LLM → Plain Text Response (with logging)
- **M2**: JSON Contract, Parsing, Validation, Blackboard Mapping
- **M3**: Behavior Tree with MoveTo, Interact, Speak actions

## M1: LLM Integration (Completed in C++)

### Features Added
- Enhanced logging in `UGeminiHTTPManager`:
  - Request URL and payload
  - Response body and status codes
  - Extracted text content
  - Failure reasons with detailed messages

- Enhanced logging in `UGeminiGenerateTextAsync`:
  - Subsystem initialization failures
  - Text extraction success/failure
  - Response text length

### Testing M1
1. Create a UAPIData DataAsset in Content Browser (Right-click → Miscellaneous → Data Asset)
2. Set the following in the DataAsset:
   - URL: `https://generativelanguage.googleapis.com/v1`
   - API Key: Your Gemini API key
   - Model: `gemini-1.5-flash` (or another Gemini model)

3. In Blueprint (Level Blueprint or Widget):
   - Add a text input box
   - Call `GenerateText` async node with:
     - WorldContextObject: `self`
     - APIData: Your created DataAsset
     - UserPrompt: Text from input box
     - Config: Create config struct with desired settings
   - Bind to OnCompleted event to handle response

4. Check Output Log for M1 logging:
   - `[GeminiHTTP] Request URL: ...`
   - `[GeminiHTTP] Request Payload: ...`
   - `[GeminiHTTP] Response (Code 200): ...`
   - `[GeminiHTTP] Extracted text: ...`

## M2: JSON Contract & Parsing (Completed in C++)

### LLM Action Contract

The LLM must output JSON in the following format:

```json
{
  "intent": "MoveTo" | "Interact" | "Speak",
  "target": {
    "id": "optional-target-id",
    "type": "optional-target-type"
  },
  "location": {
    "x": 0.0,
    "y": 0.0,
    "z": 0.0
  },
  "speak": "Text to say",
  "params": {},
  "confidence": 0.0-1.0
}
```

**Intent Whitelist**: `MoveTo`, `Interact`, `Speak`, `Idle`

### System Prompt Recommendation

Use `ULLMActionParser::GetRecommendedSystemPrompt()` in Blueprint to get the recommended system prompt. Set this in your `FGeminiGenerateContentConfig.SystemInstruction`.

Key requirements for the LLM:
- Output ONLY valid JSON
- No markdown code blocks
- No extra explanation text
- Must include `intent` and `confidence` fields
- Follow the schema for each intent type

### Using the Parser in Blueprint

**Option 1: Use the Convenience Async Node (Recommended)**

The simplest way to integrate LLM actions:

```
[GenerateAction Async Node]
  - WorldContextObject: self
  - APIData: Your Gemini API DataAsset
  - UserInput: Text from player input
  - Blackboard: AI Controller's Blackboard Component
  - Temperature: 0.7 (optional)

  → [OnCompleted Event]
    - bSuccess: true/false
    - Action: FLLMAction struct (for reference)
    - ErrorMessage: Error description if failed
```

This node automatically:
- Sets up the system prompt
- Calls the LLM with JSON-only output
- Extracts, parses, validates the JSON
- Writes to the Blackboard if valid

**Option 2: Manual Pipeline (Advanced)**

If you need more control over each step:

1. After receiving LLM response text, extract JSON:
   ```
   UGeminiHTTPManager::TryExtractStructuredJsonString(ResponseBody, JsonString)
   ```

2. Parse the JSON:
   ```
   ULLMActionParser::ParseAction(JsonString, OutAction) → returns bool, populates FLLMAction
   ```

3. Validate the action:
   ```
   ULLMActionParser::ValidateAction(Action, ErrorMessage) → returns bool
   ```

4. Normalize (optional, for named locations):
   ```
   ULLMActionParser::NormalizeAction(Action, WorldContext) → returns bool
   ```

5. Write to Blackboard:
   ```
   ULLMBlackboardMapper::WriteActionToBlackboard(Blackboard, Action, 0.5) → returns bool
   ```

**Option 3: Use ProcessLLMResponse Helper**

Single-function pipeline:
```
ULLMBlueprintLibrary::ProcessLLMResponse(LLMResponseBody, Blackboard, WorldContext, OutErrorMessage)
```

This combines steps 1-5 above into one function call.

### Validation Rules
- **Confidence**: Must be ≥ 0.5 (hardcoded threshold for MVP)
- **MoveTo**: Requires `location` (coordinates or NavPointName)
- **Interact**: Requires `target.id` or `target.type`
- **Speak**: Requires `speak` text (max 500 characters)

## M3: Behavior Tree Setup (Blueprint/Editor Tasks)

### Step 1: Create Blackboard Asset (BB_LLM)

1. Right-click in Content Browser → Artificial Intelligence → Blackboard
2. Name it `BB_LLM`
3. Add the following keys:

| Key Name        | Key Type | Description                              |
|-----------------|----------|------------------------------------------|
| Intent          | String   | Action intent: MoveTo/Interact/Speak/PlayMontage |
| TargetLocation  | Vector   | World position for MoveTo                |
| TargetActor     | Object   | Actor reference for Interact             |
| TargetId        | String   | Target identifier or NavPoint name       |
| TargetType      | String   | Target type (NPC, Door, etc.)            |
| SpeakText       | String   | Text to speak                            |
| Confidence      | Float    | Action confidence (0.0-1.0)              |
| MontageName     | String   | Animation montage name for PlayMontage   |
| MontageSection  | String   | Montage section to start from (optional) |
| MontagePlayRate | Float    | Playback rate for montage (default 1.0)  |
| MontageLoop     | Bool     | Whether to loop the montage              |

### Step 2: Create Behavior Tree (BT_LLM_MVP)

1. Right-click in Content Browser → Artificial Intelligence → Behavior Tree
2. Name it `BT_LLM_MVP`
3. Open the tree and set Blackboard Asset to `BB_LLM`

### Step 3: Build the Tree Structure

**Root Node**: Selector (or Sequence with multiple branches)

**Branch 1: MoveTo**
- Decorator: `Check Intent` (custom decorator UBTDecorator_CheckIntent)
  - IntentKey: `Intent`
  - ExpectedIntent: `MoveTo`
- Decorator: `Blackboard Based Condition`
  - Key: `TargetLocation`
  - Observer Aborts: None
- Task: `Move To` (built-in BTTask_MoveTo)
  - Blackboard Key: `TargetLocation`
  - Acceptable Radius: 100.0 or as needed

**Branch 2: Interact**
- Decorator: `Check Intent`
  - IntentKey: `Intent`
  - ExpectedIntent: `Interact`
- Decorator: `Blackboard Based Condition` (optional)
  - Key: `TargetActor` or `TargetId`
- Task: `Interact Target` (custom UBTTask_InteractTarget)
  - TargetActorKey: `TargetActor`
  - TargetIdKey: `TargetId`
  - TargetTypeKey: `TargetType`

**Branch 3: Speak**
- Decorator: `Check Intent`
  - IntentKey: `Intent`
  - ExpectedIntent: `Speak`
- Decorator: `Blackboard Based Condition`
  - Key: `SpeakText`
  - Key Query: Is Set
- Task: `Speak` (custom UBTTask_Speak)
  - SpeakTextKey: `SpeakText`
  - DisplayDuration: 3.0 seconds

**Branch 4: PlayMontage**
- Decorator: `Check Intent`
  - IntentKey: `Intent`
  - ExpectedIntent: `PlayMontage`
- Decorator: `Blackboard Based Condition`
  - Key: `MontageName`
  - Key Query: Is Set
- Task: `Play Montage` (custom UBTTask_PlayMontage)
  - MontageNameKey: `MontageName`
  - MontageSectionKey: `MontageSection`
  - MontagePlayRateKey: `MontagePlayRate`
  - MontageLoopKey: `MontageLoop`
  - Wait for Finish: false (optional)

### Step 4: Set Up AI Controller

Your AI Controller needs to:
1. Run the Behavior Tree (`Run Behavior Tree` node)
2. Have access to the Blackboard Component
3. Call the LLM action generation pipeline

**Simple Integration (Recommended):**

Use the `GenerateAction` async node in your AI Controller or Level Blueprint:

```
[User Input Event] 
  → [GenerateAction Async]
      - WorldContextObject: self
      - APIData: Your Gemini DataAsset
      - UserInput: Player's text input
      - Blackboard: AI Controller's Blackboard Component
    → [OnCompleted]
        → [Branch on bSuccess]
          → True: Action written to Blackboard, BT will execute
          → False: Log ErrorMessage
```

**Manual Integration:**

If you prefer manual control:

```
[User Input Event] 
  → [GenerateText Async] with SystemPrompt from GetLLMActionSystemPrompt
    → [OnCompleted]
      → [ProcessLLMResponse]
        - LLMResponseBody: The response from GenerateText
        - Blackboard: AI Controller's Blackboard Component
        - WorldContext: self
        → [Branch on success]
```

**Periodic/Event-Driven Execution:**

You can trigger action generation:
- On player button press
- On timer (e.g., every 5 seconds)
- On gameplay events (e.g., player enters trigger volume)
- Via UI widget text input

## Testing End-to-End

### Test Case 1: MoveTo
**Input**: "Go to coordinates 1000, 2000, 100"
**Expected LLM Output**:
```json
{"intent":"MoveTo","location":{"x":1000,"y":2000,"z":100},"confidence":0.9}
```
**Expected Behavior**:
- Parser succeeds
- Validation succeeds
- Blackboard: Intent="MoveTo", TargetLocation=(1000,2000,100)
- BT MoveTo branch executes
- Character navigates to location

### Test Case 2: Interact
**Input**: "Open the door"
**Expected LLM Output**:
```json
{"intent":"Interact","target":{"type":"Door"},"confidence":0.85}
```
**Expected Behavior**:
- Parser succeeds
- Validation succeeds
- Blackboard: Intent="Interact", TargetType="Door"
- BT Interact branch executes
- Log message: "Interacting with actor: ..." or warning about resolving target

### Test Case 3: Speak
**Input**: "Say hello to everyone"
**Expected LLM Output**:
```json
{"intent":"Speak","speak":"Hello to everyone","confidence":0.95}
```
**Expected Behavior**:
- Parser succeeds
- Validation succeeds
- Blackboard: Intent="Speak", SpeakText="Hello to everyone"
- BT Speak branch executes
- On-screen debug message appears
- Log: "[BTTask_Speak] CharacterName says: 'Hello to everyone'"

### Test Case 4: PlayMontage
**Input**: "Wave at the player"
**Expected LLM Output**:
```json
{"intent":"PlayMontage","montageName":"Wave","confidence":0.9}
```
**Expected Behavior**:
- Parser succeeds
- Validation succeeds
- Blackboard: Intent="PlayMontage", MontageName="Wave", MontagePlayRate=1.0, MontageLoop=false
- BT PlayMontage branch executes
- On-screen debug message appears
- Log: "[BTTask_PlayMontage] CharacterName playing montage: Wave (Section: None, Rate: 1.00, Loop: No)"

**Input**: "Attack with sword quickly"
**Expected LLM Output**:
```json
{"intent":"PlayMontage","montageName":"SwordAttack","montageSection":"Combo1","montagePlayRate":1.5,"confidence":0.95}
```
**Expected Behavior**:
- Parser succeeds
- Validation succeeds
- Blackboard: Intent="PlayMontage", MontageName="SwordAttack", MontageSection="Combo1", MontagePlayRate=1.5
- BT PlayMontage branch executes
- Montage plays at 1.5x speed starting from Combo1 section

### Negative Test Cases

**Low Confidence**:
```json
{"intent":"MoveTo","location":{"x":100,"y":200,"z":0},"confidence":0.3}
```
- Validation fails (below 0.5 threshold)
- Blackboard not updated
- No BT execution

**Missing Required Field**:
```json
{"intent":"Speak","confidence":0.9}
```
- Validation fails (no speak text)
- Error logged
- Blackboard not updated

```json
{"intent":"PlayMontage","confidence":0.9}
```
- Validation fails (no montageName)
- Error logged: "PlayMontage requires non-empty montageName"
- Blackboard not updated

**Invalid Play Rate**:
```json
{"intent":"PlayMontage","montageName":"Wave","montagePlayRate":10.0,"confidence":0.9}
```
- Validation fails (play rate out of range [0.1, 5.0])
- Error logged
- Blackboard not updated

**Invalid Intent**:
```json
{"intent":"Attack","confidence":0.9}
```
- Parsed as Idle (unknown intent)
- May or may not execute depending on BT structure

## Logging and Debugging

All components log to the Output Log with prefixes:
- `[GeminiHTTP]` - HTTP manager logs
- `[GeminiGenerateTextAsync]` - Async node logs
- `[LLMActionParser]` - Parser/validation logs
- `[LLMBlackboardMapper]` - Blackboard writing logs
- `[BTTask_InteractTarget]` - Interact task logs
- `[BTTask_Speak]` - Speak task logs
- `[BTTask_PlayMontage]` - PlayMontage task logs
- `[BTDecorator_CheckIntent]` - Intent checking logs

Enable verbose logging in Project Settings → Engine → General → Log Categories for detailed trace.

## Safety Constraints (MVP Notes)

- **MoveTo**: Should check if location is in navigable area (NavMesh). Current implementation uses built-in MoveTo which respects NavMesh.
- **Interact**: Should validate target is interactable. Current MVP logs interaction; extend with IInteractable interface.
- **Speak**: Limited to 500 characters. Longer text fails validation.
- **PlayMontage**: Play rate limited to [0.1, 5.0]. Montage name is required. MVP logs the montage play request; production implementation should load and play actual animation assets.
- **Confidence**: Actions below 0.5 confidence are rejected.

## Future Extensions (Out of MVP Scope)

- Additional intents: Pickup, Attack, Follow, etc.
- Service nodes to resolve TargetId → TargetActor
- Animation and audio integration for Speak
- IInteractable interface implementation
- Named navigation point registry
- Multiplayer/server authority
- Advanced safety: rate limiting, restricted areas, permission checks
- Offline replay harness for testing
- i18n preprocessing for non-English input

## Troubleshooting

**Problem**: LLM returns markdown code blocks
**Solution**: Ensure system prompt clearly states "Output ONLY valid JSON. No markdown, no explanation."

**Problem**: Parser fails with valid-looking JSON
**Solution**: Check if LLM included extra text before/after JSON. Use `TryExtractStructuredJsonString` to extract just the JSON portion.

**Problem**: Blackboard not updating
**Solution**: Check logs for validation failures. Ensure confidence ≥ 0.5 and required fields are present.

**Problem**: BT not executing
**Solution**: Verify BT is running, Blackboard asset is set, and decorators are properly configured.

**Problem**: MoveTo fails
**Solution**: Ensure target location is on NavMesh. Check Navigation → Build Navigation mesh in editor.

## API Reference

### Blueprint-Callable Functions

**ULLMBlueprintLibrary** (Convenience Helpers):
- `ProcessLLMResponse(LLMResponseBody, Blackboard, WorldContext, OutErrorMessage)` → bool
  - Complete pipeline: extract JSON, parse, validate, write to blackboard
- `GetLLMActionSystemPrompt()` → FString
  - Returns the recommended system prompt for action generation
- `GetIntentAsString(Action)` → FString
  - Convert action intent enum to string
- `IsActionValid(Action, OutErrorMessage)` → bool
  - Check if action is valid

**ULLMGenerateActionAsync** (High-Level Async Node):
- `GenerateAction(WorldContext, APIData, UserInput, Blackboard, Temperature)` → Async Action
  - OnCompleted event: (bSuccess, Action, ErrorMessage)
  - Complete end-to-end pipeline in one async node

**ULLMActionParser**:
- `ParseAction(JsonText, OutAction)` → bool
- `ValidateAction(Action, OutErrorMessage)` → bool
- `NormalizeAction(Action, WorldContext)` → bool
- `GetRecommendedSystemPrompt()` → FString

**ULLMBlackboardMapper**:
- `WriteActionToBlackboard(Blackboard, Action, ConfidenceThreshold)` → bool
- `ClearLLMKeys(Blackboard)` → void
- `GetRequiredBlackboardKeysDescription()` → FString

**UGeminiHTTPManager**:
- `TryExtractStructuredJsonString(JsonResponse, OutJsonString)` → bool (static)

### Behavior Tree Nodes

**UBTDecorator_CheckIntent**: Checks if Intent matches expected value
**UBTTask_InteractTarget**: Interacts with target actor (logs for MVP)
**UBTTask_Speak**: Displays speak text on screen and logs
**UBTTask_PlayMontage**: Plays animation montage on AI character (logs for MVP, TODO: load and play actual assets)
