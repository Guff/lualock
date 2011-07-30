require "odious"
local oocairo = require "oocairo"

-- {{{ lualock settings
style{ color = "#333333", font = "Sans 12", x = 500, y = 400, off_x = 5,
        off_y = 6, width = 150, height = 24 }

prefs{ timeout = 10 * 60 }
-- }}}

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
-- {{{ Hooks
-- get current DPMS settings
local standby, suspend, off = dpms.get()

hook.connect("lock", function () 
    dpms.set(60, 120, 300)
end)

-- restore DPMS settings once we're done
hook.connect("unlock", function ()
    dpms.set(standby, suspend, off)
end)

-- show a dot for each failed login attempt
evildot = utils.get_data_dir() .. "/glowydot.png"
failed_attempts = 0
hook.connect("auth-failed", function ()
    local dot = image(evildot)
    dot:set_position(500 + 20 * failed_attempts, 440)
    dot:show()
    failed_attempts = failed_attempts + 1
end)
-- }}}

-- {{{ 
background("color", "#000000")

im = image(utils.get_data_dir() .. "/archlinux-official-light.svg")
im:scale(0.75, 0.75)
im:set_position(0.1, 0.4)
im:show()

user_text = odious.widget.text{ text = "User: " .. os.getenv("USER"), x = 500,
                                y = 370, font = "Anton 16", color = "#ffffff",
                                border_color = "#000000", border_width = 3 }

local size = 300
local surface = cairo_surface(size, size)
local clock_bg = oocairo.surface_create_similar(surface:get_surface(), "color-alpha", size, size)

function setup_context(cr)
    -- Move (0, 0) into the center and (-1, -1) to the top-left corner)
    cr:translate(size / 2, size / 2)
    cr:scale(size / 2, size / 2)
    return cr
end

function path_hand(cr, from, to, val, max)
    local val = val + max / 2
    cr:save()
    cr:rotate(2 * math.pi * val / max)
    cr:move_to(0, from)
    cr:line_to(0, to)
    cr:restore()
end

-- Setup the clock background, create_similar already cleared the surface
local cr = oocairo.context_create(clock_bg)
setup_context(cr)

cr:push_group()

-- First the "translucent clock frame"
cr:push_group()
local rad = oocairo.pattern_create_radial(0, 0, 0, 0, 0, 1)
rad:add_color_stop_rgba(0, 1, 1, 1, 1)
rad:add_color_stop_rgba(0.9, 1, 1, 1, 1)
rad:add_color_stop_rgba(1, 0, 0, 0, 0)
cr:set_source(rad)
cr:paint()
local s = cr:pop_group()

cr:push_group()
cr:set_operator("source")
local pat = oocairo.pattern_create_linear(0, 0, 0.2, 0.9)
pat:add_color_stop_rgba(0, 0, 0, 0, 0)
pat:add_color_stop_rgba(1, 1, 1, 1, 1)
pat:set_extend("reflect")
cr:set_source(pat)
cr:paint()
cr:pop_group_to_source()
cr:mask(s)

-- Now the blue-ish background
local pat = oocairo.pattern_create_radial(-0.3, -0.4, 0, -0.2, -0.35, 0.7)
pat:add_color_stop_rgb(0, 0.9, 0.9, 1)
pat:add_color_stop_rgb(1, 0.9, 0.9, 1)
cr:set_source(pat)
cr:arc(0, 0, 0.8, 0, 2 * math.pi)
cr:fill()

-- Paint the 15min marks
local w, h = cr:device_to_user_distance(1, 1)
if w < 0 then w = -w end
if h < 0 then h = -h end
if w < h then w = h end
cr:set_line_width(w)
for i = 15, 60, 15 do
    path_hand(cr, 0.85, 1, i, 60)
end
cr:set_source_rgb(1,0,0)
cr:stroke()

-- Now make the whole background translucent
cr:pop_group_to_source()
cr:mask(oocairo.pattern_create_rgba(0, 0, 0, 0.7))

local update_clock = function()
    local time = os.date("*t")
    local cr = oocairo.context_create(surface:get_surface())
    local hour = time.hour
    local min = time.min
    local sec = time.sec

    min = min + sec / 60
    hour = hour + min / 60

    if hour > 12 then hour = hour - 12 end

    -- Paint the background
    cr:save()
    cr:set_source(clock_bg)
    cr:set_operator("source")
    cr:paint()
    cr:restore()

    setup_context(cr)

    cr:set_line_cap("round")
    cr:set_source_rgb(0,0,0)

    -- Paint the hour hand
    cr:set_line_width(0.05)
    path_hand(cr, 0, 0.4, hour, 12)
    cr:stroke()

    -- Paint the minute hand
    cr:set_line_width(0.02)
    path_hand(cr, 0, 0.65, min, 60)
    cr:stroke()

    -- Paint the second hand
    cr:set_line_width(0.01)
    path_hand(cr, 0, 0.7, sec, 60)
    cr:stroke()

    surface:show()
end
local surf_timer = timer(update_clock, 1)
surface:set_position(650, 50)
surf_timer:start()
surface:show()

update_clock()
