-- @author Julien Danjou &lt;julien@danjou.info&gt;
-- @copyright 2009 Julien Danjou

local setmetatable = setmetatable
local ipairs = ipairs
local math = math
local capi = { cairo_surface = cairo_surface, utils = utils }
local oocairo = require "oocairo"

--- A progressbar widget.
-- Adapted from awesome's awful library
module("odious.widget.progressbar")

local data = setmetatable({}, { __mode = "k" })

local properties = { "width", "height", "border_color",
                     "color", "background_color",
                     "vertical", "value", "max_value", }

local function update(pbar)
    local width = data[pbar].width or 20
    local height = data[pbar].height or 60

    -- Wipe the slate clean
    local surface = pbar.surface or capi.cairo_surface(width, height)
    surface:resize(width, height)
    cr = oocairo.context_create(surface:get_surface())
    cr:set_operator("clear")
    cr:paint()
    cr:set_operator("over")

    local value = data[pbar].value
    local max_value = data[pbar].max_value
    if value >= 0 then
        value = value / max_value
    end

    local over_drawn_width = width
    local over_drawn_height = height
    local border_width = 0
    if data[pbar].border_color then
        -- Draw border
        cr:rectangle(0.5, 0.5, width - 1, height - 1)
        cr:set_line_width(2)
        local r, g, b, a = capi.utils.parse_color(data[pbar].border_color);
        cr:set_source_rgba(r, g, b, a)
        cr:stroke()
        over_drawn_width = width - 2 -- remove 2 for borders
        over_drawn_height = height - 2 -- remove 2 for borders
        border_width = 1
    end
    
    local r, g, b, a = capi.utils.parse_color(data[pbar].color or "red")
    cr:set_source_rgba(r, g, b, a)
    cr:rectangle(border_width, border_width, over_drawn_width, over_drawn_height)
    cr:fill()

    -- Cover the part that is not set with a rectangle
    r, g, b, a =
        capi.utils.parse_color(data[pbar].background_color or "#000000")
    cr:set_source_rgba(r, g, b, a)
    cr:set_operator("source")
    if data[pbar].vertical then
        local rel_height = math.floor(over_drawn_height * (1 - value))
        cr:rectangle(border_width, border_width, over_drawn_width, rel_height)
        cr:fill()
    else
        local rel_x = math.ceil(over_drawn_width * value)
        cr:rectangle(border_width + rel_x,
                     border_width,
                     over_drawn_width - rel_x,
                     over_drawn_height)
        cr:fill()
    end

    -- Update the image
    pbar.surface = surface
    
    pbar.surface:set_position(data[pbar].x, data[pbar].y)
    pbar.surface:show()
end

--- Set the progressbar value.
-- @param pbar The progress bar.
-- @param value The progress bar value between 0 and 1.
function set_value(pbar, value)
    local value = value or 0
    local max_value = data[pbar].max_value
    data[pbar].value = math.min(max_value, math.max(0, value))
    update(pbar)
    return pbar
end

--- Set the progressbar height.
-- @param progressbar The progressbar.
-- @param height The height to set.
function set_height(progressbar, height)
    data[progressbar].height = height
    update(progressbar)
    return progressbar
end

--- Set the progressbar width.
-- @param progressbar The progressbar.
-- @param width The width to set.
function set_width(progressbar, width)
    data[progressbar].width = width
    update(progressbar)
    return progressbar
end

-- Build properties function
for _, prop in ipairs(properties) do
    if not _M["set_" .. prop] then
        _M["set_" .. prop] = function(pbar, value)
            data[pbar][prop] = value
            update(pbar)
            return pbar
        end
    end
end

--- Create a progressbar widget.
-- @param args Standard widget() arguments. You should add width and height
-- key to set progressbar geometry.
-- @return A progressbar widget.
function new(args)
    local args = args or {}
    local width = args.width or 100
    local height = args.height or 20
    
    local x, y = args.x or 0, args.y or 0

    local pbar = {}

    data[pbar] = { width = width, height = height, value = 0, max_value = 1,
                   x = x, y = y}

    -- Set methods
    for _, prop in ipairs(properties) do
        pbar["set_" .. prop] = _M["set_" .. prop]
    end

    return pbar
end

setmetatable(_M, { __call = function(_, ...) return new(...) end })
