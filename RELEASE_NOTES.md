# roo_powersafefs 1.1.1

- Fix Arduino dependency discovery by including `roo_threads.h` directly from `roo_powersafefs.h`.
- Upgrade the Bazel dependencies `roo_threads` from 1.3.1 to 1.4.0 and `roo_testing` from 2.6.0 to 2.7.0. Update the shared CI workflow to use `roo_testing` 2.7.0 as well.

---

# roo_powersafefs 1.1.0

- Upgrade the Bazel dependencies `roo_threads` from 1.2.9 to 1.3.1 and `roo_testing` from 2.3.0 to 2.6.0. Update shared CI to `roo_testing` 2.6.0 and switch it to the `arduino_esp32` profile.
- Add Bazel host-emulation profiles for Arduino and ESP-IDF on ESP32-C3 and ESP32-S3, plus Pico SDK and Arduino-Pico on RP2040, including an Arduino-Pico FreeRTOS variant. The new profiles require explicit Bazel configuration imports.
- Expand `test_all_profiles` from two to nine profiles, teach the Bazel wrapper to recognize explicit Pico SDK selections, and declare FreeRTOS as the ESP32 base profile’s runtime.

---

# roo_powersafefs 1.0.3

- Upgrade `roo_threads` from 1.2.8 to 1.2.9 and `roo_testing` from 2.1.2 to 2.3.0.
- Update Bazel tooling with an ESP-IDF ESP32 profile, automatic profile selection for ESP-IDF example runs, and a helper to test both Arduino and ESP-IDF profiles.

---

# roo_powersafefs 1.0.2

- Upgrade `roo_threads` to 1.2.8 and `roo_testing` to 2.1.2.
- Update Bazel dependencies: `rules_cc` to 0.2.25 and `googletest` to 1.18.0.bcr.1.
- Update the shared CI workflow to `roo_testing` 2.1.2.
- Add consolidated release notes.

---

# [roo_powersafefs 1.0.1](https://github.com/dejwk/roo_powersafefs/releases/tag/1.0.1)

Published 2026-08-21.

(experimental)

---

