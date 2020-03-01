--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY.
--

commandArray = {}

if (globalvariables['Security'] == 'Disarmed') then
	commandArray['OpenURL'] = 'http://192.168.xxx.xxx/?command=Disarm'
elseif (globalvariables['Security'] ~= 'Disarmed') then
	commandArray['OpenURL'] = 'http://192.168.xxx.xxx/?command=Armed'
end

return commandArray