-- Licensed under the GNU General Public License v2
--- Miscellaneous utilities for lualock
-- @author Guff &lt;cassmacguff@gmail.com&gt;
-- @copyright 2011 Guff<br>
-- 2010 Adrian C. &lt;anrxc@sysphere.org&gt;<br>
-- 2009 Rémy C. &lt;shikamaru@mandriva.org&gt;<br>
-- 2009 Benedikt Sauer &lt;filmor@gmail.com&gt;<br>
-- 2009 Henning Glawe &lt;glaweh@debian.org&gt;<br>
-- 2009 Lucas de Vries &lt;lucas@glacicle.com&gt;

local io = io
local setmetatable = setmetatable

module "odious.util"
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

-- Taken from the vicious library for awesome wm
function pathtotable(dir)
    return setmetatable({ _path = dir },
        { __index = function(table, index)
            local path = table._path .. '/' .. index
            local f = io.open(path)
            if f then
                local s = f:read("*all")
                f:close()
                if s then
                    return s
                else
                    local o = { _path = path }
                    setmetatable(o, getmetatable(table))
                    return o
                end
            end
        end
        }
    )
end

function get_battery(name)
	return pathtotable("/sys/class/power_supply/" .. name)
end
