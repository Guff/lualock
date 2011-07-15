local io = io

module("odious.util")

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

