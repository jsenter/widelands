dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "track_winter",
   -- TRANSLATORS: This track is made of footprints in the snow
   descname = _ "Track",
   editor_category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 30, 15 },
      },
   }
}
