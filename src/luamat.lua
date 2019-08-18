-------------------------------- vec2

Vec2 = {}
Vec2.__index = Vec2
Vec2Caller = {}

function Vec2.new(x_val, y_val)
	local ret = { x = x_val, y = y_val }
	setmetatable(ret, Vec2)
	return ret
end

function Vec2.from_table(tab)
	return Vec2.new(tab.x, tab.y)
end

setmetatable(Vec2, Vec2Caller)

function Vec2Caller.__call(t, x, y)
	return Vec2.new(x, y)
end

function Vec2:set(x_val, y_val)
	self.x = x_val
	self.y = y_val
end

function Vec2:clone()
	return Vec2.new(self.x, self.y)
end

function Vec2:length_square()
	return self.x * self.x + self.y * self.y
end

function Vec2:length()
	return math.sqrt(self:length_square())
end

function Vec2:normalize()
	local dem = 1 / self:length()
	return Vec2(self.x * dem, self.y * dem)
end

function Vec2.dot(lhs, rhs)
	return lhs.x * rhs.x + lhs.y * rhs.y
end

function Vec2:equals(rhs)
	return self.x == rhs.x and self.y == rhs.y
end

function Vec2.__eq(a, b)
	return a:equals(b)
end

function Vec2.__add(a, b)
	if(type(a) == "number") then
		return Vec2.new(a + b.x, a + b.y)
	end
	if(type(b) == "number") then
		return Vec2.new(a.x + b, a.y + b)
	end
	return Vec2.new(a.x + b.x, a.y + b.y)
end

function Vec2.__mul(a, b)
	if(type(a) == "number") then
		return Vec2.new(a * b.x, a * b.y)
	end
	if(type(b) == "number") then
		return Vec2.new(a.x * b, a.y * b)
	end
	return Vec2.new(a.x * b.x, a.y * b.y)
end

function Vec2.__sub(a, b)
	if(type(b) == "number") then
		return Vec2.new(a.x - b, a.y - b)
	end
	if(type(a) == "number") then
		return Vec2.new(a - b.x, a - b.y)
	end
	return Vec2.new(a.x - b.x, a.y - b.y)
end

function Vec2.__div(a, b)
	if(type(b) == "number") then
		return Vec2.new(a.x / b, a.y / b)
	end
	return Vec2.new(a.x / b.x, a.y / b.y)
end

function Vec2.__unm(v)
	return Vec2.new(-v.x, -v.y)
end

function Vec2.__tostring(vec)
	return "(" .. vec.x .. ", " .. vec.y .. ")"
end

-------------------------------- vec3

Vec3 = {}
Vec3.__index = Vec3
Vec3Caller = {}

function Vec3.new(x_val, y_val, z_val)
	local ret = { x = x_val, y = y_val, z = z_val }
	setmetatable(ret, Vec3)
	return ret
end

function Vec3.from_table(tab)
	return Vec3.new(tab.x, tab.y, tab.z)
end

setmetatable(Vec3, Vec3Caller)

function Vec3Caller.__call(t, x, y, z)
	return Vec3.new(x, y, z)
end

function Vec3:set(x_val, y_val, z_val)
	self.x = x_val
	self.y = y_val
	self.z = z_val
end

function Vec3:clone()
	return Vec3.new(self.x, self.y, self.z)
end

function Vec3:length_square()
	return self.x * self.x + self.y * self.y + self.z * self.z
end

function Vec3:length()
	return math.sqrt(self:length_square())
end

function Vec3:normalize()
	local dem = 1 / self:length()
	return Vec3(dem * self.x, dem * self.y, dem * self.z)
end

function Vec3.dot(lhs, rhs)
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z
end

function Vec3.cos(lhs, rhs)
	return Vec3.dot(lhs, rhs) / (lhs:length() * rhs:length())
end

function Vec3.cross(lhs, rhs)
	local x = lhs.y * rhs.z - lhs.z * rhs.y
	local y = lhs.z * rhs.x - lhs.x * rhs.z
	local z = lhs.x * rhs.y - lhs.y * rhs.x
	return Vector3.new(x, y, z)
end

function Vec3:equals(rhs)
	return self.x == rhs.x and self.y == rhs.y and self.z == rhs.z
end

function Vec3.__eq(a, b)
	return a:equals(b)
end

function Vec3.__add(a, b)
	if(type(a) == "number") then
		return Vec3.new(a + b.x, a + b.y, a + b.z)
	end
	if(type(b) == "number") then
		return Vec3.new(a.x + b, a.y + b, a.z + b)
	end
	return Vec3.new(a.x + b.x, a.y + b.y, a.z + b.z)
end

function Vec3.__mul(a, b)
	if(type(a) == "number") then
		return Vec3.new(a * b.x, a * b.y, a * b.z)
	end
	if(type(b) == "number") then
		return Vec3.new(a.x * b, a.y * b, a.z * b)
	end
	return Vec3.new(a.x * b.x, a.y * b.y, a.z * b.z)
end

function Vec3.__sub(a, b)
	if(type(b) == "number") then
		return Vec3.new(a.x - b, a.y - b, a.z - b)
	end
	if(type(a) == "number") then
		return Vec3.new(a - b.x, a - b.y, a - b.z)
	end
	return Vec3.new(a.x - b.x, a.y - b.y, a.z - b.z)
end

function Vec3.__div(a, b)
	if(type(b) == "number") then
		return Vec3.new(a.x / b, a.y / b, a.z / b)
	end
	return Vec3.new(a.x / b.x, a.y / b.y, a.z / b.z)
end

function Vec3.__unm(v)
	return Vec3.new(-v.x, -v.y, -v.z)
end

function Vec3.__tostring(vec)
	return "(" .. vec.x .. ", " .. vec.y .. ", " .. vec.z .. ")"
end

-------------------------------- coord

Coord = {}
Coord.__index = Coord
CoordCaller = {}

function Coord.new(x, y, z)
	local ret = {
		x = x:normalize(),
		y = y:normalize(),
		z = z:normalize()
	}
	setmetatable(ret, Coord)
	return ret
end

function Coord.from_table(tab)
	return Coord.new(tab.x, tab.y, tab.z)
end

setmetatable(Coord, CoordCaller)

function CoordCaller.__call(t, x, y, z)
	return Coord.new(x, y, z)
end

function Coord:set(x, y, z)
	self.x = x:normalize()
	self.y = y:normalize()
	self.z = z:normalize()
end

function Coord:clone()
	return Coord.new(self.x, self.y, self.z)
end

function Coord:global_to_local(v)
	return Vec3.new(Vec3.dot(self.x, v), Vec3.dot(self.y, v), Vec3.dot(self.z, v))
end

function Coord:local_to_global(v)
	return self.x * v.x + self.y * v.y + self.z * v.z
end

function Coord:in_positive_x_hemisphere(v)
	return Vec3.dot(self.x, v) > 0
end

function Coord:in_positive_y_hemisphere(v)
	return Vec3.dot(self.y, v) > 0
end

function Coord:in_positive_z_hemisphere(v)
	return Vec3.dot(self.z, v) > 0
end

-------------------------------- color

Color = {}
Color.__index = Color
ColorCaller = {}

function Color.new(r, g, b)
	local ret = { r = r, g = g, b = b }
	setmetatable(ret, Color)
	return ret
end

function Color.from_table(tab)
	return Color.new(tab.r, tab.g, tab.b)
end

setmetatable(Color, ColorCaller)

function ColorCaller.__call(t, r, g, b)
	return Color.new(r, g, b)
end

function Color:set(r, g, b)
	self.r = r
	self.g = g
	self.b = b
end

function Color:clone()
	return Color.new(self.r, self.g, self.b)
end

function Color:lum()
	return 0.2126 * self.r + 0.7152 * self.g + 0.0722 * self.b
end

function Color:equals(rhs)
	return self.r == rhs.r and self.g == rhs.g and self.b == rhs.b
end

function Color.__eq(a, b)
	return a:equals(b)
end

function Color.__add(a, b)
	if(type(a) == "number") then
		return Color.new(a + b.r, a + b.g, a + b.b)
	end
	if(type(b) == "number") then
		return Color.new(a.r + b, a.g + b, a.b + b)
	end
	return Color.new(a.r + b.r, a.g + b.g, a.b + b.b)
end

function Color.__mul(a, b)
	if(type(a) == "number") then
		return Color.new(a * b.r, a * b.g, a * b.b)
	end
	if(type(b) == "number") then
		return Color.new(a.r * b, a.g * b, a.b * b)
	end
	return Color.new(a.r * b.r, a.g * b.g, a.b * b.b)
end

function Color.__sub(a, b)
	if(type(b) == "number") then
		return Color.new(a.r - b, a.g - b, a.b - b)
	end
	if(type(a) == "number") then
		return Color.new(a - b.r, a - b.g, a - b.b)
	end
	return Color.new(a.r - b.r, a.g - b.g, a.b - b.b)
end

function Color.__div(a, b)
	if(type(b) == "number") then
		return Color.new(a.r / b, a.g / b, a.b / b)
	end
	return Color.new(a.r / b.r, a.g / b.g, a.b / b.b)
end

function Color.__unm(c)
	return Color.new(-c.r, -c.g, -c.b)
end

function Color.__tostring(c)
	return "(" .. c.r .. ", " .. c.g .. ", " .. c.b .. ")"
end

-------------------------------- distribution

Distribution = {}
Distribution.__index = Distribution

function Distribution.uniform_on_sphere(u1, u2)
	local z = 1 - 2 * u1
	local phi = 2 * math.pi * u2
	local r = math.sqrt(math.max(0, 1 - z * z))
	local x = r * math.cos(phi)
	local y = r * math.sin(phi)
	return Vec3(x, y, z), 1 / (4 * math.pi)
end

Distribution.uniform_on_sphere_pdf = 1 / (4 * math.pi)

function Distribution.uniform_on_hemisphere(u1, u2)
	local z = u1
	local phi = 2 * math.pi * u2
	local r = math.sqrt(math.max(0, 1 - z * z))
	local x = r * math.cos(phi)
	local y = r * math.sin(phi)
	return Vec3(x, y, z), 1 / (2 * math.pi)
end

Distribution.uniform_on_hemisphere_pdf = 1 / (2 * math.pi)

function Distribution.zweighted_on_hemisphere(u1, u2)
	local sam = Vec2(0, 0)
	u1 = 2 * u1 - 1
	u2 = 2 * u2 - 1
	if(u1 ~= 0 or u2 ~= 0) then
		local theta = 0
		local r = 0
		if(math.abs(u1) > math.abs(u2)) then
			r = u1
			theta = 0.25 * math.pi * (u2 / u1)
		else
			r = u2
			theta = 0.5 * math.pi - 0.25 * math.pi * (u1 / u2)
		end
		sam = r * Vec2(math.cos(theta), math.sin(theta))
	end
	
	local z = math.sqrt(math.max(0, 1 - sam:length_square()))
	return Vec3(sam.x, sam.y, z), z / math.pi
end

function Distribution.zweighted_on_hemisphere_pdf(z)
	if(z > 0) then
		return z / math.pi
	end
	return 0
end
