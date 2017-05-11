strUsage = [[
Usage: lua test_sqiflash_nxhx90.lua
]]

if #arg ~= 0 then
	print(strUsage)
	os.exit(0)
end


require("muhkuh_cli_init")
require("sqitest")

local tBus = sqitest.BUS_Spi
local uiUnit = 0
local uiChipSelect = 0
local atParameter = {
		ulInitialSpeed = 10000,
		ulMaximumSpeed = 1000,
		ulIdleCfg = 
			  sqitest.MSK_SQI_CFG_IDLE_IO1_OE + sqitest.MSK_SQI_CFG_IDLE_IO1_OUT
			+ sqitest.MSK_SQI_CFG_IDLE_IO2_OE + sqitest.MSK_SQI_CFG_IDLE_IO2_OUT
			+ sqitest.MSK_SQI_CFG_IDLE_IO3_OE + sqitest.MSK_SQI_CFG_IDLE_IO3_OUT,
		ulSpiMode = 3,
		-- LSB = CS, CLK, MISO, MSB = MOSI
		ulMmioConfiguration = 0xffffffff
	}
	


-- Open the plugin
tPlugin = tester.getCommonPlugin()
if tPlugin==nil then
	error("No plugin selected, nothing to do!")
end


-- Download the binary.
local aAttr = sqitest.download(tPlugin, "netx/", tester.progress)

atParameter.strCmpData = ""
print()
print("=======================================================")
print("=======================================================")
print()

local fOk = sqitest.sqitest(tPlugin, aAttr, tBus, uiUnit, uiChipSelect, fnCallbackMessage, fnCallbackProgress, atParameter)

print()
print("=======================================================")
print("=======================================================")
print()

-- disconnect the plugin
tPlugin:Disconnect()


if fOk then
	print("")
	print(" #######  ##    ## ")
	print("##     ## ##   ##  ")
	print("##     ## ##  ##   ")
	print("##     ## #####    ")
	print("##     ## ##  ##   ")
	print("##     ## ##   ##  ")
	print(" #######  ##    ## ")
	print("")
else
	print("")
	print("######   ####  ###### ##     ###### ####### ")
	print("##      ##  ##   ##   ##     ##      ##   ##")
	print("##      ##  ##   ##   ##     ##      ##   ##")
	print("#####   ######   ##   ##     #####   ##   ##")
	print("##      ##  ##   ##   ##     ##      ##   ##")
	print("##      ##  ##   ##   ##     ##      ##   ##")
	print("##      ##  ## ###### ###### ###### #######" )
	print("")

	error("SQI test failed")
end




