# ArduinoRFID
RFID authentication with Arduino.

# Domoticz

## Configuration

1. Set a static IP for the ESP-01s on your router.

2. Create the user variables rfid_id y rfid_ids_valid.
* rfid_id will contain the last tag sent to Domoticz.
* rfid_ids_valid will contain all valid tags.

3 Create the scripts using those found in the Domoticz_scripts directory, replace in the scripts the static IP associated with your wifi module.

## Add a new Tag

1. Scan the tag.

2. Go to Domoticz log and copy the ID.

3. Enter it in the user variable rfid_ids_valid by separating each Id by : (Ej: IDTAG1:IDTAG2:...).


# Arduino

1. Remember to replace the Domoticz IP and the user variable ID rfid_id in the code.

2. Flash Arduino and ESP-01s

3. Make the circuit using the included schemes