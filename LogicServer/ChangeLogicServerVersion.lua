local fileName = 'LogicDef.h'
local file = io.open(fileName, 'r')
local strSrc = file:read('*a')
file:close()
local strVersionPattern = '#define%s+LOGIC_KN_VERSION%s+MAKEFOURCC%s*%(%s*%d+%s*,%s*%d+%s*,%s*%d+%s*,%s*%d+%s*%)'
local strVersion = strSrc:match(strVersionPattern)
-- print(strVersion)
local y, m, d, t = strVersion:match('%(%s*(%d+)%s*,%s*(%d+)%s*,%s*(%d+)%s*,%s*(%d+)%s*%)')
-- print(y, m, d, t )
local nowM, nowD,nowY = string.match(os.date(), '(%d+)/(%d+)/(%d+)')
-- print(nowY, nowM, nowD)
if y == nowY and m == nowM and d == nowD then
	t = tonumber(t) + 1
else
	t = 1
end
local strNewVersion = strVersion:gsub('%(%s*%d+%s*,%s*%d+%s*,%s*%d+%s*,%s*%d+%s*%)', string.format('(%s, %s, %s, %d)', nowY, nowM, nowD, t))
-- print(strNewVersion)
strSrc = strSrc:gsub(strVersionPattern, strNewVersion)
-- print(strSrc)
-- print(nowY, nowM, nowD, t)
file = io.open(fileName, 'w')
file:write(strSrc)
file:close()
