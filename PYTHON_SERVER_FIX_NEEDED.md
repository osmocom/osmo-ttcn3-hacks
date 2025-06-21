# Python Server Fix Required

## File: /app/pysim/osmo-smdpp.py

### Change Required
In the `getBoundProfilePackage` method (line 785), fix the error code for downloadResponseError:

```python
# OLD (incorrect):
raise ApiError('8.11.1', '5.1', 'Profile preparation error: downloadErrorCode %s' % r_err.get('downloadErrorCode', 'unknown'))

# NEW (correct per SGP.22 v2.5):
raise ApiError('8.2.10', '4.2', 'Bound Profile Package execution error: downloadErrorCode %s' % r_err.get('downloadErrorCode', 'unknown'))
```

### Justification (SGP.22 v2.5)
- When SM-DP+ receives a `PrepareDownloadResponseError` in GetBoundProfilePackage, it should return:
  - Subject Code 8.2.10 = "Bound Profile Package"
  - Reason Code 4.2 = "Execution Error"
- The previous error code 8.11.1/5.1 is for "CI Public Key Unknown" which is unrelated to download errors
- Reference: SGP.22 v2.5 Table 32b shows 8.2.10/4.2 is used for BPP execution errors

### Test Status
- Test `TC_SM_DP_ES9_GetBoundProfilePackageNIST_04_Preparation_Error` passes with correct error codes
- Server correctly returns 8.2.10/4.2 when receiving downloadErrorCode=127 (undefinedError)

### Commit Message Suggestion
```
osmo-smdpp: Fix GetBoundProfilePackage error code for downloadResponseError

Per SGP.22 v2.5, when receiving PrepareDownloadResponseError, return
status code 8.2.10/4.2 (Bound Profile Package - Execution Error) instead
of 8.11.1/5.1 which is for unknown CI public key scenarios.

Fixes error handling to comply with specification Table 32b which shows
8.2.10/4.2 is the correct status code for BPP execution errors.
```