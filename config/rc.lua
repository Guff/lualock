require "odious"

style{ color = "#333333", font = "Sans 12", x = 500, y = 400, off_x = 5,
        off_y = 6, width = 150, height = 24 }

prefs{ timeout = 2 }
local dpms = { 
    set = function (standby, suspend, off)
        spawn(string.format("xset dpms %i %i %i", standby, suspend, off))
    end,
    get = function ()
        local xset_out = odious.util.pread("xset q")
        local standby, suspend, off =
            string.match(xset_out, "Standby: (%d+).*Suspend: (%d+).*Off: (%d+)")
        return tonumber(standby) or 0, tonumber(suspend) or 0, tonumber(off) or 0
    end 
}

local standby, suspend, off = dpms.get()

hook.connect("lock", function () 
    dpms.set(60, 120, 300)
end)
hook.connect("unlock", function ()
    dpms.set(standby, suspend, off)
end)
hook.connect("auth-failed", function () print("INTRUDER DETECTED") end)

background("color", "#000000")
im2 = image("/usr/share/archlinux/logos/archlinux-official-light.svg")
im2:scale(0.75, 0.75)
im2:set_position(0.1, 0.4)
im2:draw_rectangle(0, 0, 100, 100, true, "#ff0000")
im2:draw_line(0, 0, 0.5, 0.5, 5, "#cc0066")
im2:show()

im3 = image(100, 100)
im3:set_position(0.5, 0.5)
im3:draw_rectangle(0, 0, 0.5, 0.75, true)
im3:show()

clockbg = image(os.getenv("HOME") .. "/Pictures/clockbackground.png")
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

--function rotate_image ()
    --im2:rotate(3, im2:width() / 2, im2:height() / 2)
--end

--t = timer(rotate_image, 0.05);
