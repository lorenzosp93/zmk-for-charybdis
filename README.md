# Charybdis with Prospector

ZMK v0.3 configuration for a 35-key Charybdis with nice!nano v2 controllers,
a right-side PMW3610 trackball, and a Prospector built with a Seeed XIAO
nRF52840. Both keyboard halves are Bluetooth peripherals; the Prospector
runs the keymap and sends keyboard and mouse reports to the computer over USB.

The Prospector is configured without the optional APDS9960 light sensor,
with manually controlled screen brightness, starting at 50%. Settings-layer
controls adjust it without an ambient sensor.

## Firmware

Push this configuration to GitHub or run the Build workflow manually. The
firmware archive contains these targets:

| Board | Shield | Purpose |
| --- | --- | --- |
| `nice_nano_v2` | `charybdis_left` | Left peripheral |
| `nice_nano_v2` | `charybdis_right` | Right peripheral and trackball |
| `seeeduino_xiao_ble` | `charybdis_dongle prospector_adapter` | Prospector central (`charybdis_prospector` artifact) |
| `nice_nano_v2` | `settings_reset` | Clear either keyboard half's saved settings |
| `seeeduino_xiao_ble` | `settings_reset` | Clear the Prospector's saved settings |

Keep ZMK and the build workflow on v0.3. The pinned
[Prospector module](https://github.com/carrefinho/prospector-zmk-module)
targets this version and its `seeeduino_xiao_ble` board name.

## First-time migration and pairing

The former right-side central has saved bonds from the old topology. Clear
settings on **all three devices** before pairing the new firmware:

1. Turn off both keyboard halves.
2. Put each device into its UF2 bootloader (normally double-tap reset), flash
   the `settings_reset` firmware for its board, and let it boot to clear settings.
   The nice!nano reset image is used separately on both halves.
3. Flash the matching normal firmware onto each device. Keep both halves
   switched off while preparing the Prospector.
4. Connect the Prospector to the computer by USB. Turn on the **left half first**
   and wait for its connection to appear on the screen.
5. Turn on the **right half** and wait for the second connection.

Upstream Prospector battery widgets use pairing-slot order. This custom screen
starts with slot 1 on the left and slot 0 on the right, matching the confirmed
right-first pairing on this assembled keyboard. It infers the actual sides
from split key events as you type, and automatically corrects the display if
that order changes. After fresh pairing, press a key on either half to let the
screen learn the mapping. The inference is repeated after reboot; it does not
rewrite or clear Bluetooth bonds. Battery and disconnection indicators use
the same mapping. Physical key positions remain valid under Colemak and Studio
remapping.
The keyboard halves connect to the Prospector automatically; do not pair them
with the computer. Settings reset erases Bluetooth bonds and other persisted
settings. The existing `BT_CLR` key clears host pairing on the central and is
not a substitute for resetting all three devices during this migration.

Verify typing on both halves, mouse buttons, trackball movement, precision on
layer 4, and scrolling on layer 5. Layer names and both peripheral batteries
are displayed on the Prospector. Future keymap edits require reflashing the
Prospector; hardware/driver configuration changes require reflashing the
affected peripheral as well.

## Focused display and DPI controls

The display shows one large active-layer name, WPM at the top right, CAPS and
CW (Caps Word) at the top left, four modifier indicators, and the existing
left/right battery values and connection indicators. CTRL, ALT, GUI, and SHIFT
combine their left/right modifiers. GUI means Command on macOS or the Windows
key on Windows. CAPS reflects the lock state reported by the selected host.

On the navigation/Precision layer (hold either base-layer Z or slash key),
DPI controls use physical QWERTY positions on either base layout:

| Setting | Decrease | Increase | Steps | Boot value |
| --- | --- | --- | --- | --- |
| Normal DPI | Q or O | W or P | 400, 800, 1200, 1600 | 800 |
| Precision DPI | C or M | V or comma | 100, 200, 300, 400 | 300 |
| Prospector brightness (Settings layer) | S | D | 25%, 50%, 75%, 100% | 50% |

Steps stop at each end. These settings reset to their boot values after a
reboot. Normal and precision DPI are independent. The sensor remains at 1600
CPI; both speeds use scaling on the Prospector. Scrolling is unaffected.
End moves from O to the previously unused B position on Precision to make
room for the right-hand normal DPI controls. Other navigation keys stay put.

Alongside CAPS and CW, the screen shows both selected DPI values on every
layer, for example `DPI 800 / 300` (normal / precision). A DPI or brightness adjustment
replaces the central layer area with the selected value for three seconds.
Another press restarts the timer. The other indicators remain visible.

Enter Settings by holding a Symbols thumb key (base Tab or Esc), then holding
the other Symbols thumb key. S/D adjust the Prospector; the existing V/B
host-brightness controls still adjust the computer. The screen and backlight
turn off after approximately three minutes without keyboard or trackball
activity; activity wakes the screen and restores the selected brightness.

For an existing, paired Prospector setup, flash only the new Prospector image.
No peripheral update or settings reset is required for these display/DPI changes.

## ZMK Studio

Studio is enabled on the Prospector central. The build includes
`studio-rpc-usb-uart`, and the central selects a 35-key physical layout whose
key order matches the existing matrix transform. Its drawing is a schematic
column-staggered layout, rather than a dimensionally exact drawing of the
keyboard's curved key wells. Keyboard peripherals do not need Studio firmware.

After flashing the Prospector, connect it by USB and open
[ZMK Studio](https://zmk.studio/) in Chrome/Edge, or use the native Studio app.
Connect to the Prospector's USB serial device. On Settings, press the physical
QWERTY dot key to unlock Studio editing. Changes can be saved to the Prospector
without rebuilding firmware. Keep the unlock binding available for future edits.

The Settings A-position key is the `Caps Word / Caps Lock` tap dance:
**one tap enables Caps Word; two taps within 200 ms toggle Caps Lock**.
The CAPS indicator follows the host's lock report; CW follows Caps Word state.
Studio also exposes the custom normal/precision DPI and brightness controls,
with labelled Decrease/Increase parameters.

Studio saves mappings on the device. Once edited/saved there, those mappings
take precedence over newly flashed `.keymap` defaults; use **Restore Stock
Settings** in Studio when you want the repository's mappings again. Combo
definitions, hold-tap/tap-dance timing, DPI scales, and display code still live
in the repository. The custom QWERTY/Colemak default-layer switch from the
pinned external module remains functional, but that module does not provide
Studio parameter metadata for reassigning its controls.

## Trackball processing

The right peripheral sends full-resolution, rotated/inverted PMW3610 movement
through ZMK's `zmk,input-split` transport. The Prospector applies:

- Normal movement: adjustable scaling, initially 1/2 (800 effective CPI).
- Precision (layer 4): independent adjustable scaling, initially 3/16 (300 CPI).
  Available values are 100, 200, 300, and 400 effective CPI without rounding
  to hardware steps.
- Scroll (layer 5): map X/Y to horizontal/vertical wheel events, invert vertical
  scrolling, and accumulate one tick per 70 counts. Scroll overrides precision
  when both layers are active.

These central input processors retain fractional movement. Scroll processing
can produce both axes and multiple ticks for a large movement, whereas the old
driver emitted at most one tick and discarded the remaining counts. The sensor
now stays at 1600 CPI, so precision uses software scaling instead of changing
the hardware resolution. Confirm the feel on hardware after flashing.

`modules/pmw3610-peripheral` supplies the pinned driver's otherwise unresolved
active-layer query on peripherals, which do not compile ZMK's keymap engine.
It keeps the driver in movement mode and asserts that driver-side scroll,
precision, and automouse layers are disabled. The actual layer processing runs
on the central. `zephyr/module.yml` makes the GitHub build workflow load this
local compatibility module automatically.

For a local West build, initialize/update from `config/west.yml` and include
this repository root with `-DZMK_EXTRA_MODULES=/absolute/path/to/zmk-for-charybdis`,
alongside `-DZMK_CONFIG=/absolute/path/to/zmk-for-charybdis/config` and the board
and shield listed above.

## Validation

On 2026-09-30, all three normal firmware targets and both settings-reset targets
compiled successfully with ZMK v0.3 and Arm GNU Toolchain 13.2.Rel1. Generated
configuration checks confirmed both peripheral roles, two central peripheral
slots, matching 35-key matrix transforms, right-side trackball forwarding, and
the central's layer overrides and fixed brightness. Wireless pairing, display
operation, and trackball feel still require validation on the assembled devices.

The Focused display update compiled for all three normal firmware targets.
DPI stepping/scaling tests covered both directions, fractional accumulation,
changes between speeds, limits, and large input values with address/undefined
behavior sanitizers. Native LVGL rendering of the production screen and upstream
battery widget checked base/precision/DPI layouts, notification expiration and
restart, modifier/lock state, and backlight restoration with simulated ZMK state.
A subsequent hardware report exposed a rotation/layout error and stalled display
updates. The correction uses LVGL's logical 280 × 240 dimensions, separates the
modifier row from the battery footer, increases the graphics heap from 10 KB to
32 KB, and explicitly sets the display stack to 4 KB. Reducing the transfer
buffer to half a screen leaves firmware RAM usage at 161,444 / 262,144 bytes.

The regression fixture now uses the actual 270-degree rotation, a 32 KB graphics
heap, production screen/widget sources, and timer-driven state transitions. It
checks layer and WPM changes, DPI timeout/restart, lock/modifier state, battery
updates, row separation, and backlight restoration. ZMK events and work queues
are simulated; actual host lock reports, wireless battery updates, and physical
idle/wake still need hardware verification.

Run the portable DPI tests with:

```sh
cc -Wall -Wextra -Werror -fsanitize=undefined,address \
  modules/prospector/tests/dpi_math_test.c -o /tmp/chary-dpi-test
/tmp/chary-dpi-test
```

Run the native display regression against the pinned LVGL and Prospector
checkouts (requires CMake, a host C compiler, and Python):

```sh
cmake -S modules/prospector/tests/display -B /tmp/chary-display-test \
  -DLVGL_ROOT=/absolute/path/to/lvgl \
  -DPROSPECTOR_ROOT=/absolute/path/to/prospector-zmk-module
cmake --build /tmp/chary-display-test
ctest --test-dir /tmp/chary-display-test --output-on-failure
```

The Studio/readability build uses a 40 px coloured layer name, 28 px battery
values, 20 px WPM, 16 px lock indicators and DPI details, and 14 px headings
and modifier labels. Normal DPI notices are blue, precision notices mint,
and brightness notices amber. Healthy batteries are mint, low batteries
amber below 20%, and critical batteries red below 10%; connection indicators
remain managed by the upstream widget. The display honours stable layer IDs
and refreshes when Studio renames the currently displayed layer.

The final Studio target compiled with UART/USB and BLE Studio transports,
locking and custom-control metadata enabled. Native rendering tests additionally
checked layer renaming/reordering and larger text separation. USB serial
connection, live Studio edits/save and lock behaviour require confirmation on
the physical Prospector.

## Neon display (2026-10-02)

The WPM bar animates over 400 ms, filling at 150 WPM; higher numeric WPM
values remain visible. Width and colour move together through cyan, violet,
magenta, and orange at 0, 50, 100, and 150 WPM. The layout groups CAPS/CW and
`DPI normal / precision` in one 16 px text row above the modifier buttons.
The Prospector all-caps setting is enabled and honoured by the custom screen,
including names changed through Studio. Boot DPI defaults are now 800/300.
The original step scales, automatic half identification, Studio and idle
blanking remain enabled.
