local setmetatable = setmetatable
local ipairs = ipairs
local oocairo = require "oocairo"
local oopango = require "oopango"
local capi = { cairo_surface = cairo_surface, utils = utils }

module "odious.widget.text"

local data = setmetatable({}, { __mode = "k" })

local properties = { "text", "font", "color", "border_color", "border_width",
                     "x", "y" }
                     
local function get_extents(text)
    dummy_surf = oocairo.image_surface_create("rgb24", 1, 1)
    cr = oocairo.context_create(dummy_surf)
    layout = oopango.cairo.layout_create(cr)
    desc = oopango.font_description_from_string(data[text].font)
    layout:set_font_description(desc)
    layout:set_text(data[text].text)
    local rect = layout:get_pixel_extents()
    local width, height = rect.width, rect.height
    width = width + 2 * data[text].border_width
    height = height + 2 * data[text].border_width
    return width, height
end

local function update(text)
    local width, height = get_extents(text)

    -- Wipe the slate clean
    local surface = text.surface or capi.cairo_surface(width, height)
    surface:resize(width, height)
    cr = oocairo.context_create(surface:get_surface())
    cr:set_operator("clear")
    cr:paint()
    cr:set_operator("over")

    layout = oopango.cairo.layout_create(cr)
    desc = oopango.font_description_from_string(data[text].font)
    layout:set_font_description(desc)
    layout:set_text(data[text].text)
    cr:translate(data[text].border_width, -data[text].border_width)
    oopango.cairo.update_layout(cr, layout)
    oopango.cairo.layout_path(cr, layout)
    if data[text].border_width then
        local r, g, b, a = capi.utils.parse_color(data[text].border_color)
        cr:set_source_rgba(r, g, b, a)
        cr:set_line_join("round")
        cr:set_line_width(data[text].border_width)
        cr:stroke_preserve()
    end
    
    local r, g, b, a = capi.utils.parse_color(data[text].color)
    cr:set_source_rgba(r, g, b, a)
    cr:fill()

    -- Update the image
    text.surface = surface
    
    text.surface:set_position(data[text].x, data[text].y)
    text.surface:show()
end

-- Build properties function
for _, prop in ipairs(properties) do
    if not _M["set_" .. prop] then
        _M["set_" .. prop] = function(text, value)
            data[text][prop] = value
            update(text)
            return text
        end
    end
end

--- Create a progressbar widget.
-- @param args Standard widget() arguments. You should add width and height
-- key to set progressbar geometry.
-- @return A progressbar widget.
function new(args)
    local args = args or {}
    
    local x, y = args.x or 0, args.y or 0
    local text = args.text

    local text = {}

    data[text] = { x = x, y = y, text = args.text, color = args.color,
                   border_width = args.border_width,
                   border_color = args.border_color, font = args.font }

    -- Set methods
    for _, prop in ipairs(properties) do
        text["set_" .. prop] = _M["set_" .. prop]
    end
    
    update(text)

    return text
end

setmetatable(_M, { __call = function(_, ...) return new(...) end })

