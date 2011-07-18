--- Miscellaneous utilities for lualock
-- @author Guff &lt;cassmacguff@gmail.com&gt;
-- @copyright 2011 Guff

local io = io

module("odious.util")
--- Run a command and get its output
-- @param cmd The command to run.
-- @return Output of the command.
function pread(cmd)
    if cmd and cmd ~= "" then
        local f, err = io.popen(cmd, 'r')
        if f then
            local s = f:read("*all")
            f:close()
            return s
        else
            return err
        end
    end
end

