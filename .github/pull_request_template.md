---
name: Pull Request from dev to main
about: Merge development changes to main branch
title: 'feat: [Brief description of changes]'
labels: ['enhancement', 'dev-to-main']
assignees: []
---

## 📋 Pull Request Checklist

### Changes Made
- [ ] Bug fixes
- [ ] New features
- [ ] Documentation updates
- [ ] Configuration improvements
- [ ] Code refactoring
- [ ] Test updates

### Description
<!-- Provide a brief description of the changes -->

### Testing
- [ ] ✅ ESP8266 configuration validates successfully
- [ ] ✅ ESP32 configuration validates successfully  
- [ ] ✅ All automated tests pass
- [ ] ✅ Manual testing completed (if applicable)
- [ ] ✅ Documentation updated

### Breaking Changes
- [ ] No breaking changes
- [ ] Breaking changes (describe below)

<!-- If there are breaking changes, describe them here -->

### ESPHome Compatibility
- [ ] Compatible with ESPHome 2025.7.4+
- [ ] Tested with multiple ESPHome versions
- [ ] No API compatibility issues

### Files Modified
<!-- List the main files that were changed -->
- `components/heishamon/`
- `example-esp8266.yaml`
- `example-esp32.yaml`
- `README.md`
- `DEBUGGING.md`
- Other: 

### Hardware Testing
- [ ] Tested on ESP8266 hardware
- [ ] Tested on ESP32 hardware
- [ ] Tested with actual heat pump
- [ ] Listen-only mode tested
- [ ] Command sending tested

### Code Quality
- [ ] Code follows ESPHome conventions
- [ ] Comments are in English
- [ ] No hardcoded credentials
- [ ] Error handling implemented
- [ ] Memory usage optimized

### Documentation
- [ ] README.md updated
- [ ] DEBUGGING.md updated
- [ ] Code comments updated
- [ ] Example configurations updated

## 🔗 Related Issues
<!-- Link any related issues here -->
Closes #
Fixes #
Related to #

## 📸 Screenshots/Logs
<!-- If applicable, add screenshots or log outputs -->

## 🧪 How to Test
1. Copy the `components/` folder to your ESPHome project
2. Use the updated example configuration
3. Flash to your ESP device
4. Verify communication with heat pump

## 📝 Additional Notes
<!-- Any additional information, context, or notes -->

---

### 🔒 Merge Requirements
This PR will be automatically checked for:
- ✅ Configuration validation
- ✅ Code linting
- ✅ Documentation completeness
- ✅ Security scan
- ✅ Integration tests

**Note**: This PR should only be created from the `dev` branch to `main` branch.
