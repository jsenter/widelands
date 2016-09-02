dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Scout’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 44 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 41, 44 },
      }
   },

   aihints = {},

   working_positions = {
      atlanteans_scout = 1
   },

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "atlanteans_bread", amount = 2 },
      { name = "smoked_fish", amount = 2 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
         descname = _"scouting",
         actions = {
            "sleep=30000",
            "consume=smoked_fish",
            "worker=scout",
            "sleep=30000",
            "consume=atlanteans_bread",
            "worker=scout"
         }
      },
   },
}
