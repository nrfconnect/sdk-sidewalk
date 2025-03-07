## CI parameters

```yaml
Jenkins:
  test-sdk-sidewalk: master
  # To reconfigure functional tests:
  # use GH labels func-* (default is func-commit)
  # or
  # Use YAML Filters. Helper side to set filters: https://tester-pc.nordicsemi.no:8080/test_mgmt
  # Filters (place copied YAML filters here):
  #  - filter1:
  #     board: nrf52
```

## Description

JIRA ticket: 

## Self review

- [ ] There is no commented code.
- [ ] There are no TODO/FIXME comments without associated issue ticket.
- [ ] Commits are properly organized.
- [ ] Change has been tested.
- [ ] Tests were updated (if applicable).
