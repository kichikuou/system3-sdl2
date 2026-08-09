# Scenario tests

The scenario tests run compiled System 1, 2, and 3 scenarios through the
normal engine with SDL's dummy video and audio drivers. CTest checks both the
process exit status and the `0 tests failed` summary printed through the text
hook.

`ADISK.DAT` is checked in so that building and running the tests does not
require [sys3c](https://github.com/kichikuou/sys3c). After installing sys3c
and changing the scenario sources, regenerate it with:

```sh
for dir in system1 system2 system3 gakuen_king; do
  (cd test/scenario/$dir && sys3c)
done
```
