use("map", "common_init")

include "test_math_random"
include "test_map"
include "test_field"
include "test_immovables"
include "test_player"

include "test_ui"

include "test_table"
include "test_set"


rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit the game
   wl.debug.exit()
else
   wl.game.Player(1).see_all = true -- Reveal everything, easier for debugging
end

