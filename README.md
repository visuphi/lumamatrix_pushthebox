# Luma Matrix Push to Win Game - Zephyr Version

This repository contains the Push to Win game for the Luma Matrix. It is used at the Zephyr Meetup on 12.Februar 2026, in Winterthur as part of the competition to win the sponsored boards.

To win, the player must move the purple boxes into the blue target areas.

## Requirements

- Zepyhr 4.3.0
- Luma Matrix Shield and Raspberry Pi Pico

## build

```
west build -b rpi_pico --shield zhaw_lumamatrix
```

## flash using uf2

- If the Pico is powered on with the `BOOTSEL` button pressed, it will appear on the host as a mass storage device.

run
```
west flash --runner uf2
```

# Links

[Lumatrix Workshop ZHAW](http://www.lumatrix.fun) <br>
[Lumatrix Github Repository](https://github.com/InES-HPMM/LED-Matrix-Workshop) <br>
[Zephyr RTOS Documentation](https://docs.zephyrproject.org/latest/)
