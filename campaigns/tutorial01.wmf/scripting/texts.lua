-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- =========================
-- Some formating functions
-- =========================
-- Rich Text
function rt(text_or_opts, text)
   k = "<rt>"
   if text then
      k = ("<rt %s>"):format(text_or_opts)
   else
      text = text_or_opts
   end

   return k .. text .. "</rt>"
end

-- Headings
function h1(s)
   return "<p font=DejaVuSerif font-size=18 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=8> <br></p>"
end

function h2(s)
   return "<p font=DejaVuSerif font-size=12 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

-- Simple flowing text. One Paragraph
function p(s)
   return "<p line-spacing=3 font-size=12>" .. s .. "<br></p>" ..
      "<p font-size=8> <br></p>"
end

-- =============
-- Texts below
-- =============
scould_player = {
   title = _ "Nice and easy does it all the time",
   body = rt(p(_
[[I am sorry, but I have to rip this again. We might need the space here later
on. If I am too slow for you, you might want to play a real game and just find
everything out for yourself. Otherwise, please bear with me, I am not the
youngest and quickest anymore.]]
      )
   )
}

initial_message_01 = {
   title = _ "Welcome to the Widelands tutorial!",
   body = rt(
      h1(_"Welcome to Widelands!") ..
      p(_
[[Widelands is a slow-paced build-up strategy with an emphasis on construction,
not destruction. This tutorial will get you through the basics of the game.]]
      ) .. p(_
[[You can dismiss this box by left-clicking on the button below.]]
      )
   ),
}
initial_message_02 = {
   title = _ "Diving in",
   pos = "topright",
   field = wl.Game().map.player_slots[1].starting_field,
   body = rt(
      h1(_"Let's dive right in!") ..
      p(_
[[There are three different tribes in Widelands: the barbarians, the empire and
the atlanteans. All tribes have a different economy, strength and weaknesses,
but the general gameplay is the same for all. We play the barbarians for now.]]
      ) .. p(_
[[You usually start the game with one headquarters. This is the big building
with the blue flag in front of it. The headquarters is a warehouse that stores
wares, workers and soldiers. Some wares are needed for building houses, others
for making other wares. Obviously, the wares in the headquarters will not last
forever, so you must make sure to reproduce them. The most important wares in
the early game are the basic build wares: trunks and raw stone. Let's make sure
that we do not run out of trunks. For this, we need a lumberjack and a hut for
him to stay in.]]
      ) .. p(_
[[We need to find a nice place for the lumberjack's hut. To make this easier,
we can activate the build help. There are two ways you can do this, either by
clicking on the build help button at the bottom of the screen which is the
fourth one from the left. Or you can use the SPACE key to toggle it.]]
      ) .. p(_
[[Left-click the "OK" button to close this box and then try it.]]
      )
   ),
   obj_name = "enable_buildhelp",
   obj_title = _ "Enable the build help",
   obj_body = rt(h1(_"Enable the build help") ..
      p(_
[[It is easier to understand what is allowed to be built on which field when
the build help symbols are enabled. Do so now either by pressing SPACE or by
clicking the fourth button from the left at the very bottom of the screen.
Click the "OK" button and then give it a try.]]
      )
   )
}

lumberjack_message_01 = {
   title = _ "Lumberjack's spot",
   pos = "topright",
   field = first_lumberjack_field,
   body = rt(p(_
[[There you go. I will explain about all those symbols in a minute. First, let
me show you how to make a lumberjack's hut and how to connect it with a road.
There is a sweet spot for a lumberjack right next to those trees.  I'll describe
the steps I will take and then ask you to click on the "OK" button for me to
demonstrate.]]
      )
   )
}

lumberjack_message_02 = {
   title = _ "Building the lumberjack",
   pos = "topright",
   body = rt(p(_
[[First, I'll left-click on the symbol where I want the lumberjack's hut to be
built. A window will appear where I can choose between buildings. Because I'll
click a yellow house symbol - which means that its field can house medium and small
buildings - I am presented with all the medium buildings that I can build. The
lumberjack's hut is a small building so I will go on to select the small buildings
tab. Then I'll choose the lumberjack's hut.
Click the "OK" button to watch me. I'll go really slow: I will click - then select
the tab - and finally I'll choose the building.]]
    )
   )
}

lumberjack_message_03 = {
   title = _ "Building a connecting road",
   pos = "topright",
   body = rt(p(_
[[That won't do yet. I still need to connect the lumberjack's hut to the
rest of my road network. After I ordered a construction site, I was
automatically put in road building mode, so all I have to do is click on the
blue flag in front of my headquarters.]]
       )
   )
}

lumberjack_message_04 = {
   title = _ "Waiting for the lumberjack to go up",
   pos = "topright",
   body = rt(p(_
[[Now watch closely as a builder leaves the headquarters and goes to the
construction site. Also a carrier will take position in between the two blue
flags and carry wares from one blue flag to the other.]]
      )
   )
}

lumberjack_message_05 = {
   title = _ "Placing another flag",
   pos = "topright",
   body = rt(p(_
[[Nice how they work, isn't it? But the poor carrier has a very long way to go.
We can make it easier for him (and more efficient for us) when we place another
blue flag on the road. You try it this time: click on the yellow flag symbol
in between the two blue flags we just placed and then click on the]]
      )) .. rt("image=pics/menu_build_flag.png", p(_ "build flag symbol.")
   ),
   obj_name = "build_flag_on_road_to_lumberjack",
   obj_title = _ "Build a flag to divide the road to the lumberjack",
   obj_body = rt(h1(_"Build a flag on the road") .. p(_
[[The shorter your road segments are, the faster your wares will be transported.
You should therefore make sure that your roads have as many flags as possible.
Build a blue flag now in the middle of the road that connects your headquarters
to your lumberjack's hut.]])
   )
}

lumberjack_message_06 = {
   title = _ "Waiting for the hut to be finished",
   pos = "topright",
   body = rt(p(_
[[Well done! Let's wait till the hut is finished. If you want things to
go faster, simply use the PAGE UP key on your keyboard to increase the game
speed. You can use PAGE DOWN to make the game slower again.]]
      )
   )
}

lumberjack_message_07 = {
   title = _ "Lumberjack is done",
   pos = "topright",
   body = rt(p(_
[[Excellent. The lumberjack's hut is done. A lumberjack will now move in and
start chopping down trees, so our trunks income is secured for now. Now on to
the raw stone.]]
      )
   )
}

inform_about_stones = {
   title = _ "Some stones were found",
   body = rt(h1(_ "Getting a quarry up.")) ..
   rt(p(_
[[Stones can be mined in granite mines, but the easier way is to build a quarry
next to some stones laying around. As it happens, there is a pile of them
laying just to the west (left) of your headquarters. I will teach you now how to
move your view over there.]]
      ) .. p(_
[[There are two ways to move your view. The first one is using the cursor keys
on your keyboard. Go ahead and try this out.  Click the "OK" button and then move
the view using the cursor keys]]
      )
   ),
   obj_name = "move_view_with_cursor_keys",
   obj_title = _ "Move your view with the cursor keys",
   obj_body = rt(h1(_"Moving your view") .. p(_
[[Moving your view is essential to get a complete overview of your whole
economy. There are two ways to move your view in Widelands. The first one is
to use the cursor keys on your keyboard. The second one is the more common and
faster one: press-and-hold the right mouse button anywhere on the map, then move
your mouse around and you'll see the view scroll.]]
      )
   )
}

tell_about_right_drag_move = {
   title = _ "Other ways to move the view",
   body = rt(p(_
[[Excellent. Now there is a faster way to move, using the mouse instead: Simply
right-click-and-hold anywhere on the map, then drag the mouse and instead
of the cursor, the view will be moved. Try it.]]
      )
   ),
   obj_name = "move_view_with_mouse",
   obj_title = _ "Move your view with the mouse",
   obj_body = inform_about_stones.obj_body,
}

congratulate_and_on_to_quarry = {
   title = _ "Onward to the quarry",
   body = rt(p(_
[[Great. Now about that quarry...]]
      )
   )
}

order_quarry_recap_how_to_build = {
   field = first_quarry_field,
   pos = "topright",
   title = _ "How to build a quarry",
   body = rt(p(_
[[Build a quarry next to those stones here. Remember how I did it earlier?
Make sure that build help is on, then just click the place were you want the
building to be, choose it from the window that appears and it is placed. Maybe
it is a good time to explain about all those build help symbols we activated
earlier.]]
   ) .. p(_
[[You can build four things on fields in Widelands: Flags, small houses, medium
houses and big houses. But not every field can hold anything. The build help
eases recognition:]]
   )) .. rt("image=pics/big.png", p(_
[[Everything can be built on the green house symbol.]]
   )) .. rt("image=pics/medium.png", p(_
[[Everything except for big buildings can be built on a yellow house symbol.]]
   )) .. rt("image=pics/small.png", p(_
[[Red building symbols can only hold small buildings and flags.]]
   )) .. rt("image=pics/set_flag.png", p(_
[[And finally the yellow flag symbol only allows for flags.]]
   )) .. rt(p(_
[[If you place something on a field, the surrounding fields might have less
 space for holding buildings, so choose your fields wisely.]]
   )) .. rt(p(_
[[Now go ahead, try it. The quarry is a small building, so if you click on a
medium or big building symbol, you will have to select the small buildings
tab first to find it. Go on, check it out!]]
      )
   ),
   obj_name = "build_a_quarry",
   obj_title = _ "Build a quarry next to the stones",
   obj_body = rt(h1(_ "Build a quarry") .. p(_
[[There are some stones to the west of your headquarters. Build a quarry right
next to them. The quarry is a small building like the lumberjack's hut. You
can therefore build it on any field that shows a red, yellow or green house
when the build help is enabled (Press SPACE for that).]]) .. p(_
[[Just click on any house symbol next to the stones, select the small buildings
tab in the window that opens up, then click on the quarry symbol.]]
      )
   )
}

talk_about_roadbuilding_00 = {
   pos = "topright",
   field = wl.Game().map:get_field(9,12),
   title = _ "Road building",
   body = rt(p(_
[[Excellent! Directly after you placed a building, you are in road building
mode. The new road will start at the flag in front of your newly placed
construction site. You can enter road building mode for any flag by
left-clicking on a flag and selecting]]
      )) .. rt("image=pics/menu_build_way.png", p(_
[[the road building symbol.]]
      )) .. rt(p(_
[[If you decide you do not want to build a road at this time, you can cancel
road building by clicking on the starting flag of the road and selecting]]
      )) .. rt("image=pics/menu_abort.png", p(_
[[the abort symbol.]]
      )) .. rt(p(_
[[Now, about this road. Remember: we are already in road building mode since you
just ordered the quarry. You can either make it longer by one field at a time
by left-clicking multiple times on neighbouring fields for perfect control over
the route the road takes like so:]]
      ))
}

talk_about_roadbuilding_01 = {
   pos = "topright",
   field = wl.Game().map:get_field(9,12),
   title = _ "Road building",
   body = rt(p(_
[[Alternatively, you can directly click the flag where
the road should end like so.]]
   ))
}

talk_about_roadbuilding_02 = {
   pos = "topright",
   title = _ "Road building",
   body = rt(p(_
[[One more thing: around the field where your road would end you can see
different markers. They have the following meaning:]]
      )) .. rt("image=pics/roadb_green.png", p(_
[[The terrain is flat here. Your carriers will be very swift on this terrain.]]
   )) .. rt("image=pics/roadb_yellow.png", p(_
[[There is a small slope to climb to reach this field. This means your
workers are faster walking downwards than they are walking upwards.]]
   )) .. rt("image=pics/roadb_red.png", p(_
[[The connection between the fields is extremely steep. The speed increase in
one direction is huge while the slowdown in the other is also substantial.]]
   )) .. rt(p(_
[[Keep the slopes in mind while placing roads and use them to your advantage.
Also try to keep roads as short as possible and always remember to place as
many flags as you can on road segments to share the load better.]]
   )) .. rt(p(_
[[Now please rebuild the road between your quarry and your headquarters.
We'll wait until the quarry is completed.]]
   )),
   obj_name = "build_road_to_quarry",
   obj_title = _ "Connect the quarry to the headquarters",
   obj_body = rt(h1(_"Connect your construction site") .. p(_
[[Connect your quarry construction site to your headquarters with a road. You
are directly in road building mode when you ordered a new site. But now, you
aren't. To build a completely new road just click on the flag in front of your
construction site, click on the build road icon then click on the flag in front
of your headquarters. Wait for the completion of the stonemason's hut.]]
      )
   )
}

census_and_statistics_00 = {
   title = _ "Census and statistics",
   body = rt(p(_
[[While we wait, I'll quickly show you another useful feature. All construction
sites look the same and some buildings look alike. It is sometimes hard to tell
them apart. Widelands offers a feature to show label texts over the buildings.
They are called the census and you can toggle them via the 'c' key or via
the button on the watch tab of any field.]]
    ) .. p(_
[[Similar to this are building statistics which are also toggled via a
button on the watch tab of any field. The hotkey for it is 's'. This will
display an information string about the productivity of buildings or the
progress of construction sites.]]
   ) .. p(_
[[Let me quickly enable those two for you. Remember: 'c' and 's' are the keys.
Alternatively you can click on any field without a building on it, select the
watch tab and then click on the corresponding buttons.]]
   )
)
}

census_and_statistics_01 = {
   title = _ "Census and statistics",
   body = rt(p(_
[[Now we know what's going on. Let's wait for this quarry to finish.]]
   )
)
}

teaching_about_messages = {
   popup = true,
   title = _ "Introducing messages",
   body = rt(h1(_"Messages") ..
      p(_
[[Hi, it's me again! This time, I sent you a message. Messages are sent to you
by Widelands to inform you about important events: empty mines, attacks on your
tribe, won or lost military buildings, resources found...]]
      ) .. p(_
[[The message window can be toggled by the button on the very right at the
bottom of the screen. This button also changes appearance if new messages are
available, but there is also a bell sound played whenever you receive a new
message.]]
      ) .. p(_
[[Currently, you have two messages. This one which you are currently reading and
the one that informed you that a new headquarters was added to your economy.
Let's learn how to archive messages: You can check them off in your inbox so
that they get a tick-symbol in front of them. Then, you can click the]]
      )) .. rt("image=pics/message_archive.png", p(_
[[archive message button to move them into your archive.]]
      )) .. rt(p(_
[[Archive all messages, including this one, that you currently have in your
 inbox.]]
      )
   ),
   obj_name = "archive_all_messages",
   obj_title = _"Archive all messages in your inbox",
   obj_body = rt(h1(_"Archive our inbox messages") .. p(_
[[The message window is central to fully controlling your tribe's fortune. But
you get a lot of messages in a real game. To keep your head straight, you should
try to keep the inbox empty. Archive all your messages in the inbox now. To do
so, open the messages window by pressing 'n' or clicking the right most button
at the very bottom of the screen. Then mark all messages by checking the check
box in front of them. Then, click the archive all button]]
      )
   )
}

closing_msg_window_00 = {
   pos = "topright",
   field = first_quarry_field,
   title = _"Closing windows",
   body = rt(p(_
[[Excellent. By the way: closing windows in Widelands is as easy as
right-clicking on them. This works with all windows except for story message
windows like this one. Go ahead and try it. First, close this window by pressing
the button below, then right click into the messages window to close it.]]
      )
   ),
   obj_name = "close_message_window",
   obj_title = _ "Close the messages window",
   obj_body = rt(h1(_"Close the messages window") .. p(_
[[All windows in Widelands can be closed by right clicking into them. Some
windows can also be toggled with the buttons at the very bottom of the screen.
Close the messages window now by right clicking into it.]]
      )
   )
}

closing_msg_window_01 = {
   pos = "topright",
   field = first_quarry_field,
   title = _ "Closing windows",
   body = rt(p(_
[[Well done! Let's see how messages work in the real game, shall we? For this,
I'll take all stones away from the poor stonemason in the quarry. He will then
send a message that he can't find any in his working area the next time he
tries to do some work.]]
      )
   )
}

conclude_messages = {
   pos = "topright",
   title = _ "Message arrived!",
   body = rt(p(_
[[A message has been sent to you. See how the button at the bottom of the
screen has changed appearance? You might now burn this quarry down as it is
no longer of any use and is just blocking space. To do that, click on the
quarry and select the destroy button.]]
   ))
}

introduce_expansion = {
   title = _ "Expanding your territory!",
   body = rt(p(_
[[There is one more thing I'd like to teach you now: Expanding your territory.
The place that we start with around our headquarters is barely enough for a
basic build infrastructure and we do not have access to mountains which we
need to mine minerals and coal. So we have to expand our territory.]]
      ) .. p(_
[[Expanding is as simple as building a military building at the corner of
your territory. The barbarians have a few different military buildings:
sentries, barriers, donjons, fortresses and citadels. The bigger
the building, the more expensive it is to build, the more land it conquers
around itself and the more soldiers can be stationed there. The buildings also
vary in their vision range: buildings with a tower see farther than others.]]
      ) .. p(_
[[As soon as a military building is manned, it extends your land. You can then
burn it down again if you need the place. But note that your land is then
vulnerable: any military site from another player can conquer the land. You
therefore need military sites to keep military influence over your land.]]
      ) .. p(_
[[Let's try it out now: Build a few military buildings on your south western
border.  We want to capture some of this mountain, so we can search for
resources there. Bigger buildings will conquer more land which can be beneficial
close to mountains because you can't build houses in mountains.]]
      )
   ),
   obj_name = "conquer_mountain",
   obj_title = _ "Conquer an area were we can build mines",
   obj_body = rt(h1(_"Conquer the mountain to the south west") .. p(_
[[For a full-fledged economy, we need coal, iron and gold. These can be found
in mountains. Conquer some area on the mountains to the south-west of your
headquarters by building some military buildings close to your border.]]
   ) .. p(_
[[You can choose from the following military buildings: sentry, 
donjon, barrier and fortress. The bigger the building, the more expensive it is
to be built. But it will also conquer a bigger region. Sometimes, it is useful
to build a big military building next to a mountain so that you can reach
fields farther up.]]
      )
   )
}

mining_00 = {
   pos = "topright",
   title = _ "Searching for resources",
   body = rt(p(_
[[Okay, now we own some of the area on this mountain. Mountains are very
important, because they contain resources: coal, iron ore, gold ore and
granite.  Each tribe uses the resources differently, but all need mines to get
the resources out of the ground.]]
   ) .. p(_
[[Let's search for resources in this mountain.  First, we'll build a road into
the mountains and place a flag. Then, we click on the flag and call a geologist
to it. I'll show you how it's done.]]
   )
)
}

mining_01 = {
   pos = "topright",
   title = _"Waiting for the geologist",
   body = rt(p(_
[[The geologist will arrive shortly to the flag and start investigating the
area in his surroundings. He will place the following markers for the various
resources:]]
   )) ..
   rt("image=tribes/barbarians/resi_coal1/resi_00.png", p(_ "a bit of coal")) ..
   rt("image=tribes/barbarians/resi_coal2/resi_00.png", p(_ "a lot of coal")) ..
   rt("image=tribes/barbarians/resi_iron1/resi_00.png", p(_ "a bit of iron")) ..
   rt("image=tribes/barbarians/resi_iron2/resi_00.png", p(_ "a lot of iron")) ..
   rt("image=tribes/barbarians/resi_gold1/resi_00.png", p(_ "a bit of gold")) ..
   rt("image=tribes/barbarians/resi_gold2/resi_00.png", p(_ "a lot of gold")) ..
   rt("image=tribes/barbarians/resi_granit1/resi_00.png",
      p(_ "a bit of granite")) ..
   rt("image=tribes/barbarians/resi_granit2/resi_00.png",
      p(_ "a lot of granite")) ..
   rt("image=tribes/barbarians/resi_water1/resi_00.png", p(_ "water")) ..
   rt("image=tribes/barbarians/resi_none/resi_00.png",
      p(_ "nothing was found here"))
   .. rt(p(_
[[Let's wait and see what the geologist finds on the mountain.]]
   )
)
}

mining_02 = {
   pos = "topright",
   title = _ "Mining conclusion",
   body = rt(p(_
[[So our geologist found a lot of coal on this mountain. You should therefore
build a coal mine here. Building a mine is like building a house. The build help
symbol for where a mine can be built is]]
   )) .. rt("image=pics/mine.png", p(_"this one.")) ..
   rt(p(_
[[Note that a mine needs rations to work. Rations are
produced in taverns and taverns need meat, pitta bread, and fish to produce
them. You will need a lot more infrastructure to get your mines working
well. This infrastructure also varies from tribe to tribe. You'll get them
explained in the introduction campaigns for the three tribes.]]
   )
)
}

warfare_and_training_00 = {
   title = _ "Warfare and Training",
   body = rt(h1(_ "Soldiers and Warfare") .. p(_
[[On to the last topic now. We are going to talk about soldiers, their training
and their profession: warfare. As mentioned, Widelands is about building up,
not burning down: therefore warfare is also more focused on the economics than
the military strategies. Nevertheless, warfare offers one way to challenge
other players and it has some game mechanics that deserve explanation. The
economies of the tribes are explained in their individual tutorial campaigns.
Ok, I am going to create us a little training ground with a training camp and a
warehouse to the north east of here.]]) .. p(_
[[If you want to come back to this south-western part of your realm, just scroll here
via right-button scrolling or open the minimap by clicking on the]]
         )) .. rt("image=pics/menu_toggle_minimap.png", p(_
[[minimap button at the bottom of the screen. Alternatively you could also press
'm' on your keyboard]]
         )) .. (rt(p(_
[[The minimap shows you the complete map in miniature. You can directly jump to
any field by left-clicking on it. You can also toggle buildings, roads, flags
and player indicators on and off inside the minimap.]]
         ) .. p(_
[[But I digress. Back to soldiers. What was I about to do? Oh yes, I wanted to
build a small training scenario for you. Let's do that now.]]
         )
      )
   )
}

warfare_and_training_01 = {
   pos = "topright",
   title = _ "Trainings camp and soldier stats",
   body = rt(p(_
[[There we go. Take a look at the soldiers that are on their way into our
trainings camp. They look different than normal workers: they have a health bar
over their head that displays their remaining hitpoints and they have four
symbols which symbolize the individual soldier's current levels in the four
different categories hitpoints, attack, defense and evade.]]
      ) .. p(_
[[A soldier is created as any normal worker: a carrier grabs a tool in a
warehouse as soon as a request for a certain profession is not fulfilled. The
tool to create a barbarian soldier is an axe. The newly created soldier is of
level 0. To make the soldier better in any of the four categories, training is
needed. Training happens in training sites like the trainings camp or the
battle arena: soldiers go there (as our little fellows are currently doing),
consume some wares and advance a level in one category. If a barbarian soldier
is fully trained, he has level 3 hitpoints, level 5 attack, level 0 defense and
level 2 evade. This is one fearsome warrior then! The individual statistics
have the following meaning:]]
      ) .. h2(_"Hitpoints:") .. p(_
[[The total life of a soldier. A barbarian soldier starts with ~130 hitpoints,
with each hitpoint level he gains 28 hitpoints.]]
      ) .. h2(_"Attack:") .. p(_
[[The amount of damage a soldier inflicts upon a successful attack on the
enemy. A barbarian soldier with attack level 0 inflicts ~14 hitpoints damage
when he succeeds to hit an enemy. For each attack level, he gains 7 damage.]]
      ) .. h2(_"Defense:") .. p(_
[[Defense is the value that is subtracted from the attack value. The barbarians
can not train in this skill and therefore have always defense level 0 which
means that they always get 3 hitpoints subtracted from the damage inflicted. If
an attacker with an attack value of 15 hitpoints hits a barbarian soldier, the
barbarian would lose 15 - 3 = 12 hitpoints. The 3 hitpoints that are subtracted
are because of the defense ability.]]
      ) .. h2(_"Evade:") .. p(_
[[Evade is the chance that the soldier is able to dodge an attack. It is 25% for
a level 0 evade barbarian and increases in steps of 15% for each level.]]
      )
   )
}

enhance_fortress = {
   pos = "topright",
   title = _ "Enhance this fortress",
   body = rt(h1(_ "Enhancing buildings") .. p(_
[[I will create an enemy for you soon, but let's make sure you are prepared.
This fortress is already quite strong and conquers a lot of space. But there is
an even bigger building: the citadel.]]
      ) .. p(_
[[Citadels can not be built directly. Instead, you have to construct a fortress
first and then enhance it to a citadel. To do so, click on the fortress, then
choose the enhance to citadel button. Your soldiers will leave the citadel
while the construction is going on. This means that your fortress has no
military influence any more. If an enemy builds a military building nearby,
your construction site could burn down. No sweat, that won't happen here.]]
      ) .. p(_
[[Enhance your fortress to a citadel now. Remember that you can speed time up
by using PAGE_UP, building a citadel takes a while.]]
      )
   ),
   obj_name = "enhance_fortress",
   obj_title = _"Enhance your fortress to a citadel",
   obj_body = rt(h1(_ "Enhance your fortress") .. p(_
[[Enhance your fortress to a mighty citadel. The citadel can house 12 soldiers
and is the biggest military building the barbarians can build. It also costs a
lot and takes a long time to build. It is most suited to guard strategically
important points like constricted points or mountains.]]
      )
   )
}

attack_enemey = {
   pos = "topright",
   title = _ "Defeat your enemy",
   body = rt(h1(_ "Defeat the enemy") .. p(_
[[I created a sparring partner for you: It is an empire tribe close to your
citadel. To attack its buildings, click on the door of your target building,
choose the number of soldiers that you wish to send and click on the attack
button. Your soldiers will come from all nearby military buildings. Likewise,
the defenders will come from all nearby military buildings of the enemy and
intercept your forces.]]
      ) .. p(_
[[Attack and conquer all military buildings of the enemy and destroy its
headquarters.]]
      )
   ),
   obj_name = "defeated_the_empire",
   obj_title = _ "Defeat the enemy tribe",
   obj_body = rt(h1(_"Defeat your enemy") ..  p(_
[[Defeat the nearby enemy. To attack a building, click on its doors, choose the
number of attacking soldiers, then send them via the attack button.]]
      )
   )
}

conclude_tutorial = {
   title = _ "Conclusion",
   body = rt(h1(_"Conclusion") ..
      p(_
[[This concludes the tutorial. There is some stuff we have not covered here --
we have not even built a single producing building even though producing wares
is the most important thing in Widelands -- but you've learned the ropes. You
can learn about the remaining stuff while you go through the individual tribe's
introduction campaigns. Each consists of some scenarios explaining the tribes
and their economy while introducing the background story of Widelands. Have fun
playing!]]
      ) .. p(_
[[You can continue playing this map or you can end this game whenever you like.
To leave this game and return to the main menu click on the]]
      )) .. rt("image=pics/menu_options_menu.png", p(_
[[options menu button on the very left at the bottom of the screen.
Then click the]]
      )) .. rt("image=pics/menu_exit_game.png", p(_
[[exit game button.]]
      )) .. rt(p(_
[[Thanks for playing this tutorial. Enjoy Widelands and remember
to visit us at]]
      )) .. rt("text-align=center",
   "<p font-size=24 font-decoration=underline>http://www.widelands.org</p>"
   )
}



