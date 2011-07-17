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
    dpms.set(1, 2, 3)
end)
hook.connect("unlock", function ()
    dpms.set(standby, suspend, off)
end)

background("color", "#ff0000")
background("/usr/share/backgrounds/Aeg_by_Tauno_Erik.jpg", "zoom")
im2 = image("/usr/share/archlinux/logos/archlinux-official-light.svg")
im2:rotate(60)
im2:scale(.75, .75)
im2:show(0.5, 0.5)
im2:draw_rectangle(0, 0, 100, 100, true, "#

user_text = text{ text = "User: " .. os.getenv("USER"), x = 500, y = 370,
                  font = "Anton 16", color = "#ffffff", border_color = "#000000",
                  border_width = 3 }
user_text:draw()

text_clock = text{ text = os.date("%a %b %d, %l:%M:%S %p"), x = 0, y = 0,
                   font = "Sans 20", color = "#ffffff", border_color = "#000000",
                   border_width = 4 }
text_clock:draw()

clock_timer = timer(function ()
    text_clock:set{ text = os.date("%a %b %d, %l:%M:%S %p") }
    text_clock:draw()
end, 1)

text_timer_text = text{ text = "Locked for 0 seconds", x = 100, y = 300, 
                        font = "Sans 12", color = "#ffffff",
                        border_color = "#000000", border_width = 6 }
text_timer_text:draw()
start_time = os.time()
text_timer = timer(function ()
	text_timer_text:set{ text = string.format("Locked for %i seconds", os.time() - start_time) }
    text_timer_text:draw()
end, 1)

function rotate_image ()
    im2:rotate(3, im2:width() / 2, im2:height() / 2)
end

t = timer(rotate_image, 0.05);
