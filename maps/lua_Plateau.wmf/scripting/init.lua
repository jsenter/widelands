-- =======================================================================
--                             The Green Plateau                            
-- =======================================================================
use("aux", "coroutine")
use("aux", "infrastructure")
use("aux", "table")
use("aux", "smooth_move")

p1 = wl.game.Player(1)
p2 = wl.game.Player(2)
p3 = wl.game.Player(3)
p4 = wl.game.Player(4)

use("map", "texts")
use("map", "initial_conditions")

-- Main mission thread
function mission_thread()
   sleep(300)

   send_msg(briefing_1_the_forbidden_island)

   while not p1:seen_field(wl.map.Field(5,8)) do sleep(2345) end

   -- Unhide the castle and keep it unhidden so that it can be attacked
   local castle = wl.map.Field(23, 19)
   p1:reveal_fields(castle:region(4))

   -- Show the way to the castle very briefly
   local way = array_combine(
      wl.map.Field( 5,  8):region(2), 
      wl.map.Field( 9, 10):region(2), 
      wl.map.Field(12, 12):region(2), 
      wl.map.Field(15, 14):region(2), 
      wl.map.Field(18, 16):region(2), 
      wl.map.Field(20, 17):region(2)
   )
   p1:reveal_fields(way)
   sleep(100)
   p1:hide_fields(way)

   -- Move to the castle
   smooth_move(castle)
   sleep(1000)
   send_msg(briefing_2_found_ancient_castle)
   local o = add_obj(obj_capture_ancient_castle)

   -- Wait till we conquered the castle
   while #p1:get_buildings"castle.atlanteans" < 1 do sleep(2345) end
   o.done = true

   smooth_move(castle)
   sleep(1000)
   
   p1:reveal_fields(castle:region(18))
   send_msg(briefing_3_captured_ancient_castle)
   local o_erwyn = add_obj(obj_defeat_erwyn)
   local o_jomo = add_obj(obj_defeat_jomo) 

   sleep(100)
   p1:hide_fields(castle:region(18))

   -- Function to check for victory over erwyn
   run(function()
      while not p2.defeated do sleep(3434) end
      send_msg(briefing_erwyn_defeated)
      o_erwyn.done = true
   end)
   -- Function to check for victory over jomo
   run(function()
      while not p3.defeated do sleep(3434) end
      send_msg(briefing_jomo_defeated)
      o_jomo.done = true
   end)

   while not (p2.defeated and p3.defeated) do sleep(4325) end

   send_msg(last_briefing_victory)
end

run(mission_thread)
