--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY.
--

commandArray = {}

for variableName,variableValue in pairs(uservariablechanged) do
    if (variableName == 'rfid_id' and string.find(uservariables['rfid_ids_valid'], variableValue)) then
        commandArray['Domoticz Security Panel'] = 'Disarm'
        commandArray['OpenURL'] = 'http://192.168.xxx.xxx/?command=Disarm'
        print('RFID_Tag_checker: Acceso concedido a: ' .. variableValue)
    elseif (variableName == 'rfid_id' and globalvariables['Security'] == 'Disarmed') then
        commandArray['OpenURL'] = 'http://192.168.xxx.xxx/?command=Disarm'
        print('RFID_Tag_checker: Acceso denegado a: ' .. variableValue)
    elseif (variableName == 'rfid_id') then
        commandArray['OpenURL'] = 'http://192.168.xxx.xxx/?command=Armed'
        print('RFID_Tag_checker: Acceso denegado a: ' .. variableValue)
    end
end

return commandArray