--
-- TraCI Dissector for Wireshark
-- Copyright (C) 2013 Christoph Sommer <christoph.sommer@uibk.ac.at>
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; either version 2
-- of the License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
--

traci_proto = Proto("TraCI","TraCI (Traffic Control Interface)")
function traci_proto.dissector(buffer, pinfo, tree)

	pinfo.cols.protocol = "TraCI"

	local msgOffset = 0
	while (msgOffset < buffer:len()) do

		local msg = buffer(msgOffset)

		if (msg:len() < 4) then
			-- buffer contains less than 4 Bytes (which we need for reading the message length) -> request to be called again with more packets concatenated to the buffer
			pinfo.desegment_offset = msgOffset
			pinfo.desegment_len = 4 - msg:len()
			return nil
		end

		local messageLen = msg(0,4):uint()

		if (msg:len() < messageLen) then
			-- buffer contains less Bytes than message length -> request to be called again with more packets concatenated to the buffer
			pinfo.desegment_offset = msgOffset
			pinfo.desegment_len = messageLen - msg:len()
			return nil
		end

		--
		-- buffer contains (at least) one message -> start parsing message
		--

		local subtreeMsg = tree:add(traci_proto, msg(0, messageLen), "TraCI Message of length " .. messageLen)
		subtreeMsg:add(msg(0, 4), "Message length: " .. messageLen)

		cmdOffset = 4
		while (cmdOffset < messageLen) do
			-- read command length
			local commandLen = tonumber(msg(cmdOffset + 0, 1):uint())
			local commandLenExt = 0
			local cmdStartOffset = 1
			if commandLen == 0 then
				cmdStartOffset = 5
				commandLenExt = tonumber(msg(cmdOffset + 1, 4):uint())
			end
			local commandId = tonumber(msg(cmdOffset + cmdStartOffset, 1):uint())

			local subtreeCmd = subtreeMsg:add(traci_proto, msg(cmdOffset + 0, commandLen + commandLenExt), "TraCI Command 0x" .. string.format("%X", commandId))
			subtreeCmd:add(msg(cmdOffset + 0, 1), "Command length: " .. commandLen)
			if commandLenExt > 0 then
				subtreeCmd:add(msg(cmdOffset + 1, 4), "Command length ext: " .. commandLenExt)
			end
			subtreeCmd:add(msg(cmdOffset + cmdStartOffset, 1), "Command id: 0x" .. string.format("%X", commandId))
			if commandLen + commandLenExt - cmdStartOffset - 1 > 0 then
				subtreeCmd:add(msg(cmdOffset + cmdStartOffset + 1, commandLen + commandLenExt - cmdStartOffset - 1), "Data of length " .. commandLen + commandLenExt - cmdStartOffset - 1)
			end

			-- a CMD_SIMSTEP2 returned from the server gets special treatment: it is immediately followed by an (unframed) count of returned subscription results
			local wasSimstep = (commandId == 2) and (commandLen == 7) and (tonumber(msg(cmdOffset + cmdStartOffset + 1, 1):uint()) == 0)
			if wasSimstep then
				local numSubscriptions = tonumber(msg(cmdOffset + commandLen + commandLenExt, 4):uint())
				subtreeMsg:add(msg(cmdOffset + commandLen + commandLenExt, 4), "Number of subscription results: " .. numSubscriptions)
				cmdOffset = cmdOffset + 4
			end

			-- end of command, retry with rest of bytes in this message
			cmdOffset = cmdOffset + commandLen + commandLenExt
		end


		--- end of message, retry with rest of bytes in this packet
		msgOffset = msgOffset + messageLen
	end

end

tcp = DissectorTable.get("tcp.port")
tcp:add(9999,traci_proto)

-- end
