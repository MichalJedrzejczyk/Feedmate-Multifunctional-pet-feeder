# User Manual

## Starting the Device

After powering on the device, the LCD displays the main FeedMate screen.

## Controls

| Key | Function |
|---|---|
| A | Confirm selected option / start feeding / repeat action |
| B | Move to previous menu item / edit opening time from start screen |
| C | Move to next menu item |
| D | Open menu / go back |
| # | Show status / clear input while editing |
| * | Delete last digit while editing |

## Main Functions

### Manual Feeding

Press `A` on the start screen to start manual feeding.

### Menu Navigation

Press `D` to open the menu.  
Use `B` and `C` to move through menu options.  
Press `A` to select an option.

### Opening Time Configuration

The opening time defines how long the servo mechanism stays open during feeding.

The value is entered in seconds and can be configured from the menu.

### Automatic Feeding Mode

Automatic feeding can be enabled or disabled from the menu.

The feeding interval can be configured in seconds. For example:

- `60` means every 1 minute
- `3600` means every 1 hour
- `86400` means every 24 hours

### Sound Feedback

Sound feedback can be enabled or disabled from the menu.

## Current Limitations

- Settings are not saved after power loss
- The device does not currently use Wi-Fi
- The device does not currently include a mobile application
- Feeding is interval-based, not clock-based

## Planned Improvements

- ESP32-based version
- Mobile application
- Wi-Fi connectivity
- Remote feeding activation
- Feeding history
- Food level monitoring
