local guy = Image.toTexture(Data.Sprites.Test)
local gradient = Image.toTexture(Data.Sprites.Test2)
local width, height, x, y, rotation, speed = 0, 0, 0, 0, 0, 0
local left, down = true, true

local derp = Object.new()
derp:makeDrawable(0, 0, 0, 0, guy)

function onResize()
	width = Display.width
	height = Display.height
	speed = math.floor(Display.width / 200)
end

onResize()

function onDraw()
	if left then
		x = x + speed
	else
		x = x - speed
	end
	
	if down then
		y = y + speed
	else
		y = y - speed
	end
	
	if x > width - 128 then
		left = false
	elseif x < 0 then
		left = true
	end
	
	if y > height - 128 then
		down = false
	elseif y < 0 then
		down = true
	end
	
	rotation = rotation + 4
	guy:draw(x, y, 128, 128, 64, 64, rotation)
	gradient:draw(0, 0, height, height)
	if rotation >= 360 then
		rotation = 0
	end
end
