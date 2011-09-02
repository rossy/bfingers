local function makedatanode(name)
	local ret, size, data = {}, -1
	setmetatable(ret, { __index = function(table, key)
		if key == "name" then
			return name
		elseif key == "size" then
			if size == -1 then
				data, size = Gar.get(name)
			elseif size == 0 then
				return
			end
			return size
		elseif key == "data" then
			if size == -1 then
				data, size = Gar.get(name)
			elseif size == 0 then
				return
			end
			return data
		elseif name then
			return makedatanode(name .. "." .. key)
		else
			return makedatanode(key)
		end
	end } )
	return ret
end
Data = makedatanode() 

function onDraw()
	
end

function onResize()
	
end
