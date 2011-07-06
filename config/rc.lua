style.set({ color = "#333333", font = "Sans 12", x = 500, y = 560, off_x = 5,
            off_y = 6, width = 150, height = 24 })

background.set("color", "#ff0000")
background.set("/usr/share/backgrounds/Aeg_by_Tauno_Erik.jpg", "zoom")
im2 = image.new("/usr/share/archlinux/logos/archlinux-official-light.svg")
image.rotate(im2, 60)
image.render(im2)

user_text = text.new("User: " .. os.getenv("USER"), 500, 500, "Sans 16",
                     "#666666")
text.draw(user_text);

text_clock = text.new(os.date("%a %b %d, %l:%M:%S %p"), 0, 0, "Sans 16", "#000000")
text.draw(text_clock)
clock_timer = timer.new(function ()
    text.set(text_clock, os.date("%a %b %d, %l:%M:%S %p"))
end, 1)

function rotate_image ()
    image.rotate(im2, 3)
    --image.render(im2)
end

t = timer.new(rotate_image, 0.05);
