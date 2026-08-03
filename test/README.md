# Scenario tests

The scenario tests run compiled System 1, 2, and 3 scenarios through the
normal engine with SDL's dummy video and audio drivers. CTest checks both the
process exit status and the `0 tests failed` summary printed through the text
hook.

`ADISK.DAT` is checked in so that building and running the tests does not
require [sys3c](https://github.com/kichikuou/sys3c). After installing sys3c
and changing the scenario sources, regenerate it with:

```sh
for system in system1 system2 system3; do
  (cd test/scenario/$system && sys3c)
done
```
