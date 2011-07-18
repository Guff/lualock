require "odious"

style{ color = "#333333", font = "Sans 12", x = 500, y = 400, off_x = 5,
        off_y = 6, width = 150, height = 24 }

prefs{ timeout = 10 * 60 }

-- define functions for setting and restoring DPMS settings
local dpms = { 
    set = function (standby, suspend, off)
        utils.spawn(string.format("xset dpms %i %i %i", standby, suspend, off))
    end,
    get = function ()
        local xset_out = odious.util.pread("xset q")
        local standby, suspend, off =
            string.match(xset_out, "Standby: (%d+).*Suspend: (%d+).*Off: (%d+)")
        return tonumber(standby) or 0, tonumber(suspend) or 0, tonumber(off) or 0
    end 
}

-- get current DPMS settings
local standby, suspend, off = dpms.get()

hook.connect("lock", function () 
    dpms.set(60, 120, 300)
end)

-- restore DPMS settings once we're done
hook.connect("unlock", function ()
    dpms.set(standby, suspend, off)
end)

evildot = utils.get_data_dir() .. "/glowydot.png"
failed_attempts = 0
hook.connect("auth-failed", function ()
	local dot = image(evildot)
	dot:set_position(500 + 20 * failed_attempts, 440)
	dot:show()
	failed_attempts = failed_attempts + 1
end)

background("color", "#000000")
im = image(utils.get_data_dir() .. "/archlinux-official-light.svg")
im:scale(0.75, 0.75)
im:set_position(0.1, 0.4)
im:show()

clockbg = image(utils.get_data_dir() .. "/clockbackground.png")
clockbg:show()

user_text = text{ text = "User: " .. os.getenv("USER"), x = 500, y = 370,
                  font = "Anton 16", color = "#ffffff", border_color = "#000000",
                  border_width = 3 }
user_text:draw()

clock_hr = text { x = 55, y = 40, font = "Droid Sans Mono 110",
                  color = "#ffffff", border_color = "#000000", border_width = 4 }
clock_min = text { x = 305, y = 40, font = "Droid Sans Mono 110",
                   color = "#ffffff", border_color = "#000000", border_width = 4 }
clock_noon = text { x = 495, y = 118, font = "Droid Sans Mono 50",
                    color = "#ffffff", border_color = "#000000", border_width = 3 }

clock_timer = timer(function ()
    clock_hr:set { text = os.date("%I") }
    clock_min:set { text = os.date("%M") }
    clock_noon:set { text = os.date("%p") }
    clock_hr:draw()
    clock_min:draw()
    clock_noon:draw()
end, 1)
