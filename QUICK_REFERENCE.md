# Quick Reference: LLM Action System

## Simplest Usage (One Node)

```blueprint
[GenerateAction Async]
  - WorldContextObject: self
  - APIData: MyGeminiAPI_DataAsset
  - UserInput: "Go to the fountain"
  - Blackboard: MyAIController.BlackboardComponent
  - Temperature: 0.7
  
→ [OnCompleted]
    - bSuccess: true/false
    - Action: FLLMAction (reference)
    - ErrorMessage: "..." if failed
```

## JSON Contract

```json
{
  "intent": "MoveTo|Interact|Speak",
  "target": {"id": "...", "type": "..."},
  "location": {"x": 0, "y": 0, "z": 0} or "NavPointName",
  "speak": "Text to say",
  "params": {},
  "confidence": 0.0-1.0
}
```

## Required Blackboard Keys (BB_LLM)

| Key            | Type   | Purpose                        |
|----------------|--------|--------------------------------|
| Intent         | String | MoveTo/Interact/Speak/Idle     |
| TargetLocation | Vector | MoveTo destination             |
| TargetActor    | Object | Interact target                |
| TargetId       | String | Target identifier/NavPoint     |
| TargetType     | String | Target type (NPC, Door, etc)   |
| SpeakText      | String | Text to speak                  |
| Confidence     | Float  | Action confidence (0.0-1.0)    |

## Behavior Tree Structure (BT_LLM_MVP)

```
Root: Selector
├─ Branch 1 (MoveTo)
│  ├─ CheckIntent (ExpectedIntent: "MoveTo")
│  ├─ BlackboardBasedCondition (Key: TargetLocation)
│  └─ MoveTo (built-in, Key: TargetLocation)
│
├─ Branch 2 (Interact)
│  ├─ CheckIntent (ExpectedIntent: "Interact")
│  ├─ BlackboardBasedCondition (Key: TargetActor or TargetId)
│  └─ InteractTarget (Custom)
│
└─ Branch 3 (Speak)
   ├─ CheckIntent (ExpectedIntent: "Speak")
   ├─ BlackboardBasedCondition (Key: SpeakText)
   └─ Speak (Custom)
```

## Validation Rules

- **Confidence**: Must be ≥ 0.5
- **MoveTo**: Requires location (coordinates or name)
- **Interact**: Requires target.id or target.type
- **Speak**: Requires text, max 500 characters

## Example User Inputs → Expected JSON

**"Go to coordinates 1000, 2000, 100"**
```json
{"intent":"MoveTo","location":{"x":1000,"y":2000,"z":100},"confidence":0.9}
```

**"Open the door"**
```json
{"intent":"Interact","target":{"type":"Door"},"confidence":0.85}
```

**"Say hello"**
```json
{"intent":"Speak","speak":"Hello","confidence":0.95}
```

## Blueprint Functions

### ULLMBlueprintLibrary
- `ProcessLLMResponse(ResponseBody, Blackboard, WorldContext, OutError)` → bool
- `GetLLMActionSystemPrompt()` → FString
- `IsActionValid(Action, OutError)` → bool

### ULLMActionParser
- `ParseAction(JsonText, OutAction)` → bool
- `ValidateAction(Action, OutError)` → bool
- `GetRecommendedSystemPrompt()` → FString

### ULLMBlackboardMapper
- `WriteActionToBlackboard(Blackboard, Action, Threshold)` → bool
- `ClearLLMKeys(Blackboard)` → void

## Custom Behavior Tree Nodes

- **UBTDecorator_CheckIntent**: Check if Intent matches expected value
- **UBTTask_InteractTarget**: Interact with target (logs for MVP)
- **UBTTask_Speak**: Display text on screen and log

## Setup Steps

1. **Create API DataAsset** (UAPIData)
   - URL: `https://generativelanguage.googleapis.com/v1`
   - API Key: Your Gemini key
   - Model: `gemini-1.5-flash`

2. **Create Blackboard** (BB_LLM)
   - Add 7 keys as listed above

3. **Create Behavior Tree** (BT_LLM_MVP)
   - Set blackboard to BB_LLM
   - Build 3-branch structure as shown

4. **Wire AI Controller**
   - Use GenerateAction async node
   - Connect to user input event
   - Pass Blackboard reference

5. **Run BT in AI Controller**
   - Use "Run Behavior Tree" node
   - Select BT_LLM_MVP

## Logging Prefixes

- `[GeminiHTTP]` - HTTP manager
- `[GeminiGenerateTextAsync]` - Async node
- `[LLMActionParser]` - Parser/validation
- `[LLMBlackboardMapper]` - Blackboard updates
- `[BTTask_InteractTarget]` - Interact task
- `[BTTask_Speak]` - Speak task
- `[BTDecorator_CheckIntent]` - Intent checks

## Troubleshooting

**Q: LLM returns markdown?**
A: System prompt should say "Output ONLY valid JSON. No markdown."

**Q: Parser fails?**
A: Use `TryExtractStructuredJsonString` to extract JSON from response.

**Q: Blackboard not updating?**
A: Check logs for validation failures. Ensure confidence ≥ 0.5.

**Q: BT not executing?**
A: Verify BT is running, decorators configured correctly.

**Q: MoveTo fails?**
A: Ensure target is on NavMesh (Build Navigation mesh in editor).

## References

- **Full Setup Guide**: `LLM_AI_SETUP.md`
- **Implementation Details**: `IMPLEMENTATION_SUMMARY.md`
