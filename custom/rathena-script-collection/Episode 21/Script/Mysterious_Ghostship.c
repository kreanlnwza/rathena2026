1@wtgs,1,1,0	script	#EP21_GS_control	-1,{
	end;

OnInstanceInit:
	set_instance_var("map$", instance_mapname("1@wtgs"));
	set_instance_var("mps", 0);
	set_instance_var("event_control", 0);
	set_instance_var("summon", 0);
	set_instance_var("ds", 0);
	set_instance_var("control$",instance_npcname(strnpcinfo(0)));
	instance_enable("#ep21_rw10",true);
end;

OnEvent:
	set_instance_var("event_control",get_instance_var("event_control") + 1);
	instance_event(strnpcinfo(0),"OnEvent" + (get_instance_var("event_control") > 9 ? get_instance_var("event_control") : ("0" + get_instance_var("event_control"))),false);
end;

OnEvent01:
	set_instance_var("mps", 1);
	instance_enable("Maristella Walter#ep21_rwi01",true);
	npctalk "Maristella Walter : This place is definitely something I remember. But I'm not completely sure.",instance_npcname("Maristella Walter#ep21_rwi01");
end;

OnEvent02:
	set_instance_var("mps", 2);
	instance_enable("Maristella Walter#ep21_rwi01",false);
	instance_enable("Yohan#ep21_rw04",false);
	instance_enable("Tan#ep21_rw02",false);
	instance_enable("Iana Operta#ep21_rwi01",false);
	instance_enable("Sternlight#ep21_rw01",true);
	instance_enable("Mast Traces#ep21_rw02",true);
	instance_enable("Old Box#ep21_rw03",true);
	instance_enable("Crumpled Documents#ep21_rw",true);
	instance_enable("Rusty Fountain Pen#ep21_rw05",true);
	instance_enable("Box Under a Bed#ep21_rw0",true);
	instance_event(strnpcinfo(0),"OnSummon",false);
end;

OnEvent03:
	set_instance_var("mps", 9);
	instance_enable("Maristella Walter#ep21_rwi02",false);
	instance_enable("Yohan#ep21_rw05",false);
	instance_enable("Tan#ep21_rw03",false);
	instance_enable("Iana Operta#ep21_rwi02",false);
	instance_enable("Ghostship Captain#ep21_rw01",true);
end;

OnEvent04:
	set_instance_var("mps", 10);
	instance_enable("Ghostship Captain#ep21_rw02",true);
end;

OnEvent05:
	set_instance_var("mps", 11);
	instance_enable("Howell Megas Walter#ep21_r",true);
	instance_enable("Iana Operta#ep21_rwi04",true);
	instance_enable("Tan#ep21_rw05",true);
	instance_enable("Yohan#ep21_rw07",true);
	instance_enable("Maristella Walter#ep21_rwi04",true);
	mapwarp get_instance_var("map$"),get_instance_var("map$"),309,25;
	instance_enable("Ghostship Captain#ep21_rw02",false);
	instance_enable("Maristella Walter#ep21_rwi03",false);
	instance_enable("Yohan#ep21_rw06",false);
	instance_enable("Tan#ep21_rw04",false);
	instance_enable("Iana Operta#ep21_rwi03",false);
end;

OnEvent06:
	set_instance_var("mps", 12);
	instance_enable("Howell Megas Walter#ep21_r",false);
	instance_enable("Maristella Walter#ep21_rwi04",false);
	instance_enable("Yohan#ep21_rw07",false);
	instance_enable("Tan#ep21_rw05",false);
	instance_enable("Iana Operta#ep21_rwi04",false);
	instance_enable("Maristella Walter#ep21_rwi05",true);
	instance_enable("Iana Operta#ep21_rwi05",true);
	instance_enable("Tan#ep21_rw06",true);
	instance_enable("Yohan#ep21_rw08",true);
	instance_enable("#ep21_rwarp06",true);
end;

OnEvent07:
	set_instance_var("mps", 13);
	instance_enable("#ep21_rwarp07",true);
end;

OnSummon:
	.@var = get_instance_var("summon") + 1;
	.@event$ = get_instance_var("control$") + "::OnMobKill";
	set_instance_var("summon", .@var);
	switch(.@var){
		case 1:
			.@map$ = get_instance_var("map$");
			setarray .@mob$,
				67,314,22369,"Ghostship Crew",
				67,312,22369,"Ghostship Crew",
				67,308,22369,"Ghostship Crew",
				64,310,22369,"Ghostship Crew",
				64,313,22369,"Ghostship Crew",
				61,312,22369,"Ghostship Crew";
			break;

		case 2:
			.@map$ = get_instance_var("map$");
			setarray .@mob$,
				67,329,22369,"Ghostship Crew",
				67,327,22369,"Ghostship Crew",
				70,330,22369,"Ghostship Crew",
				70,328,22369,"Ghostship Crew";
			break;

		case 3:
			.@map$ = get_instance_var("map$");
			setarray .@mob$,
				114,315,22369,"Ghostship Crew",
				117,317,22369,"Ghostship Crew",
				114,311,22369,"Ghostship Crew",
				110,310,22369,"Ghostship Crew",
				110,307,22369,"Ghostship Crew";
			break;

		case 4:
			.@map$ = get_instance_var("map$");
			setarray .@mob$,
				47,24,22369,"Ghostship Crew",
				52,25,22369,"Ghostship Crew",
				47,27,22369,"Ghostship Crew",
				52,22,22369,"Ghostship Crew",
				52,29,22369,"Ghostship Crew";
			break;

		case 5:
			.@map$ = get_instance_var("map$");
			setarray .@mob$,
				//RIGHT
				185,94,22369,"Ghostship Crew",
				185,98,22369,"Ghostship Crew",	
				185,101,22369,"Ghostship Crew",
				//CENTER
				152,98,22369,"Ghostship Crew",
				151,101,22369,"Ghostship Crew",
				149,98,22369,"Ghostship Crew",
				147,101,22369,"Ghostship Crew",
				149,94,22369,"Ghostship Crew",
				//LEFT
				111,119,22369,"Ghostship Crew",
				111,114,22369,"Ghostship Crew",
				107,114,22369,"Ghostship Crew",
				107,109,22369,"Ghostship Crew",
				111,109,22369,"Ghostship Crew",
				107,119,22369,"Ghostship Crew",
				110,104,22369,"Ghostship Crew";
			break;
	}
	for(.@i = 0; .@i < getarraysize(.@mob$); .@i += 4)
		monster .@map$,atoi(.@mob$[.@i]),atoi(.@mob$[.@i+1]),.@mob$[.@i+3],atoi(.@mob$[.@i+2]),1,.@event$;
end;

OnMobKill:
	if(mobcount(get_instance_var("map$"),get_instance_var("control$") + "::OnMobKill")) end;
	switch(get_instance_var("summon")){
		case 1:
		case 2:
			instance_event(strnpcinfo(0),"OnSummon",false);
			break;

		case 3:
			instance_enable("#ep21_rwarp01",true);
			instance_enable("#ep21_rwarp02",true);
			instance_event(strnpcinfo(0),"OnSummon",false);
			break;

		case 4:
			instance_enable("#ep21_rwarp03",true);
			instance_enable("#ep21_rwarp04",true);
			instance_event(strnpcinfo(0),"OnSummon",false);
			break;

		case 5:
			instance_enable("#ep21_rwarp05",true);
			break;
	}
end;

OnBossSummon:
	if(get_instance_var("daily") != true)
		monster get_instance_var("map$"),170,25,"Ghostship Captain",22370,1,instance_npcname(strnpcinfo(0)) + "::OnBossKill";
	else
		monster get_instance_var("map$"),170,25,"Ghostship Captain",22372,1,instance_npcname(strnpcinfo(0)) + "::OnBossKill";
end;

OnBossKill:
	if(get_instance_var("daily") != true)
		instance_event("#EP21_GS_control","OnEvent",false);
	else {
		set_instance_var("ds",3);
		instance_enable("Maristella Walter#ep21_rwi03",true);
		instance_enable("Maristella Walter#ep21_rwi04",true);
		instance_enable("Maristella Walter#ep21_rwi05",true);
		instance_enable("#ep21_rwarp06",true);
		instance_enable("#ep21_rwarp07",true);
	}
end;
}

1@wtgs,86,307,0	script(DISABLED)	#ep21_rw10	HIDDEN_WARP_NPC,4,4,{
	end;
	
OnTouch:
	if(is_party_leader() && get_instance_var("daily") == true){
		instance_enable("Maristella Walter#ep21_rwdaily",true);
		end;
	}
	if(is_party_leader() && get_instance_var("mps") == 0)
		instance_event("#EP21_GS_control","OnEvent",false);
end;
}

//Event 1
1@wtgs,84,311,3	script(DISABLED)	Maristella Walter#ep21_rwdaily	4_EP21_MARISTELLA,{
	if(is_party_leader() && isbegin_quest(16826) == 1 && !get_instance_var("ds")){
		npctalk "Maristella Walter : It is truly repeating again・・. Let's go disperse the mana.";
		set_instance_var("ds",1);
		instance_event("#EP21_GS_control","OnSummon",false);
		sleep 500;
		instance_enable("Maristella Walter#ep21_rwdaily",false);
		end;
	}
	if(is_party_leader() && !get_instance_var("ds")){
		cutin "ep21_maristella03.png",1;
		mes "[Maristella Walter]";
		mes "The wizard was right. It was exactly as he thought, it is repeating.";
		npctalk "Maristella Walter : The wizard was right. It was exactly as he thought, it is repeating.";
		next;
		mes "[Maristella Walter]";
		mes "Even if these are replicas・・ It somehow feels like staying here wouldn't give them peace.";
		npctalk "Maristella Walter : Even if these are replicas・・ It somehow feels like staying here wouldn't give them peace.";
		next;
		cutin "ep21_maristella01.png",1;
		mes "[Maristella Walter]";
		mes "Perhaps this is why I'm here myself. Now, let us press on.";
		npctalk "Maristella Walter : Perhaps this is why I'm here myself. Now, let us press on.";
		instance_enable("Maristella Walter#ep21_rwdaily",false);
		close2;
		cutin "",255;
		set_instance_var("ds",1);
		instance_event("#EP21_GS_control","OnSummon",false);
	}
	end;
}

1@wtgs,84,311,3	script(DISABLED)	Maristella Walter#ep21_rwi01	4_EP21_MARISTELLA,{
	if(is_party_leader() && get_instance_var("mps") == 1){
		instance_enable("Yohan#ep21_rw04",true);
		instance_enable("Tan#ep21_rw02",true);
		instance_enable("Iana Operta#ep21_rwi01",true);
		cutin "ep21_maristella01.png",1;
		mes "[Maristella Walter]";
		mes "From what I see・・ the ship's appearance is somewhat familiar. Though it's hard to recognize because of the wear and decay.";
		npctalk "Maristella Walter : From what I see・・ the ship's appearance is somewhat familiar. Though it's hard to recognize because of the wear and decay.";
		next;
		mes "[Maristella Walter]";
		mes "Alright, let's proceed cautiously.";
		npctalk "Maristella Walter : Alright, let's proceed carefully.";
		next;
		cutin "ep21_iana01.png",1;
		mes "[Iana Operta]";
		mes "Yes yes, I've heard that it didn't attack our ships, but something might happen if we go in further.";
		npctalk "Iana Operta : Yes yes, I've heard that it didn't attack our ships, but something might happen if we go in further.",instance_npcname("Iana Operta#ep21_rwi01");
		next;
		cutin "ep21_maristella01.png",1;
		mes "[Maristella Walter]";
		mes "Exactly. Be cautious as we move forward and look for clues that might confirm that this ship is the Walter.";
		npctalk "Maristella Walter : Exactly. Be cautious as we move forward and look for clues that might confirm that this ship is the Walter.";
		next;
		cutin "ep21_yohan01.png",2;
		mes "[Yohan]";
		mes "Hm, a ship like this is the walter・・ I can't believe it!!";
		npctalk "Yohan : Hm, a ship like this is the walter・・ I can't believe it!!",instance_npcname("Yohan#ep21_rw04");
		next;
		cutin "ep21_yohan03.png",2;
		mes "[Yohan]";
		mes "There's no way the captain and crew would survive in such a horrible state・・!";
		npctalk "Yohan : There's no way the captain and crew would survive in such a horrible state・・!",instance_npcname("Yohan#ep21_rw04");
		next;
		cutin "ep21_iana02.png",1;
		mes "[Iana Operta]";
		mes "Sigh, Yohan. I understand how you feel, but calm down.";
		npctalk "Iana Operta : Sigh, Yohan. I understand how you feel, but calm down.",instance_npcname("Iana Operta#ep21_rwi01");
		next;
		cutin "ep21_tan01.png",1;
		mes "[Tan]";
		mes "A lowly sailor raising his voice like that. The discipline of the Walter's crew is evident.";
		npctalk "Tan : A lowly sailor raising his voice like that. The discipline of the Walter's crew is evident.",instance_npcname("Tan#ep21_rw02");
		next;
		cutin "ep21_iana03.png",1;
		mes "[Iana Operta]";
		mes "Oh dear, commander Tan are you that worried about preserving the dignity of our captain?";
		npctalk "Iana Operta : Oh dear, commander Tan are you that worried about preserving the dignity of our captain?",instance_npcname("Iana Operta#ep21_rwi01");
		next;
		cutin "ep21_tan02.png",1;
		mes "[Tan]";
		mes "・・Think what you like.";
		npctalk "Tan : ・・Think what you like.",instance_npcname("Tan#ep21_rw02");
		next;
		cutin "ep21_iana02.png",1;
		mes "[Iana Operta]";
		mes "Have you heard that, Yohan? Be a little more cautios";
		npctalk "Iana Operta : Have you heard that, Yohan? Be a little more cautios",instance_npcname("Iana Operta#ep21_rwi01");
		next;
		cutin "ep21_maristella01.png",1;
		mes "[Maristella Walter]";
		mes "・・Anyway, let's keep moving forward while checking the area.";
		npctalk "Maristella Walter : ・・Anyway, let's keep moving forward while checking the area.";
		instance_event("#EP21_GS_control","OnEvent",false);
		close3;
	}
	end;
}

1@wtgs,81,312,5	script(DISABLED)	Yohan#ep21_rw04	4_EP21_YOHAN,{
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Hm, this ship can't be the walter・・ impossible. I don't believe it!";
	close3;
}

1@wtgs,86,313,3	script(DISABLED)	Tan#ep21_rw02	4_EP21_TAN,{
	cutin "ep21_tan02.png",1;
	mes "[Tan]";
	mes "Hey, Captain of the Walter. You better keep your crew in line.";
	close3;
}

1@wtgs,89,309,3	script(DISABLED)	Iana Operta#ep21_rwi01	4_EP21_IANA,{
	cutin "ep21_iana03.png",1;
	mes "[Iana Operta]";
	mes "Our commander is so kind, isn't he~";
	close3;
}

//= Event 1
1@wtgs,48,320,3	script(DISABLED)	Sternlight#ep21_rw01	4_POINT_RED,3,3,{
	if(!is_party_leader()) end;
	instance_enable("Maristella Walter#ep21_rwi06",true);
	instance_enable("Yohan#ep21_rw09",true);
	instance_enable("Tan#ep21_rw07",true);
	instance_enable("Iana Operta#ep21_rwi06",true);
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "The sternlight・・ It feels familiar.";
	npctalk "Maristella Walter : The sternlight・・ It feels familiar.",instance_npcname("Maristella Walter#ep21_rwi06");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "It wasn't this blue before・・. I was a crew of the captain's ship before・・ so I know it well.";
	npctalk "Yohan : It wasn't this blue before・・. I was a crew of the captain's ship before・・ so I know it well.",instance_npcname("Yohan#ep21_rw09");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "I agree, Yohan. The sternlight on the Walter from my memories isn't this kind of blue. It was a much warmer color.";
	npctalk "Maristella Walter : I agree, Yohan. The sternlight on the Walter from my memories isn't this kind of blue. It was a much warmer color.",instance_npcname("Maristella Walter#ep21_rwi06");
	next;
	mes "[Maristella Walter]";
	mes "But the position and shape feels familiar, which is why it's worth considering.";
	npctalk "Maristella Walter : But the position and shape feels familiar, which is why it's worth considering.",instance_npcname("Maristella Walter#ep21_rwi06");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "・・.";
	npctalk "Yohan : ・・.",instance_npcname("Yohan#ep21_rw09");
	next;
	cutin "ep21_iana01.png",1;
	mes "[Iana Operta]";
	mes "Alright, we can't be certain just by looking at the sternlight. Let's head further inside the ship.";
	npctalk "Iana Operta : Alright, we can't be certain just by looking at the sternlight. Let's head further inside the ship.",instance_npcname("Iana Operta#ep21_rwi06");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "'The blue three-eyed monster'. The Walter was never a ship that would be called something like that.";
	npctalk "Maristella Walter : 'The blue three-eyed monster'. The Walter was never a ship that would be called something like that.",instance_npcname("Maristella Walter#ep21_rwi06");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Absolutely・・!";
	npctalk "Yohan : Absolutely・・!",instance_npcname("Yohan#ep21_rw09");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Yohan, as Iana said, we can't be sure yet. We're only at the deck. But・・ It felt nostalgic.";
	npctalk "Maristella Walter : Yohan, as Iana said, we can't be sure yet. We're only at the deck. But・・ It felt nostalgic.",instance_npcname("Maristella Walter#ep21_rwi06");
	next;
	mes "[Maristella Walter]";
	mes "Let's go in further.";
	npctalk "Maristella Walter : Let's go in further.",instance_npcname("Maristella Walter#ep21_rwi06");
	instance_enable("Sternlight#ep21_rw01",false);
	instance_enable("Maristella Walter#ep21_rwi06",false);
	instance_enable("Yohan#ep21_rw09",false);
	instance_enable("Tan#ep21_rw07",false);
	instance_enable("Iana Operta#ep21_rwi06",false);
	set_instance_var("mps", get_instance_var("mps") + 1);
	close3;

OnTouch:
	if(is_party_leader())
		npctalk "!!!";
end;
}

1@wtgs,52,316,1	script(DISABLED)	Yohan#ep21_rw09	4_EP21_YOHAN,{
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "The Walter's sternlight it wasn't this・・ blue.";
	close3;
}

1@wtgs,52,319,3	script(DISABLED)	Iana Operta#ep21_rwi06	4_EP21_IANA,{
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "It isn't exactly how I remember it~. But I recognize some familiar features here and there.";
	close3;
}

1@wtgs,49,316,1	script(DISABLED)	Maristella Walter#ep21_rwi06	4_EP21_MARISTELLA,{
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Calling it a Blue three-eyed monster・・ doesn't feels right to me.";
	close3;
}

1@wtgs,48,318,7	script(DISABLED)	Tan#ep21_rw07	4_EP21_TAN,{
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "Well, I can't say much since I don't really know what the Walter Merchants ship looked like in the past.";
	close3;
}

//= Event 2
1@wtgs,112,325,3	script(DISABLED)	Mast Traces#ep21_rw02	4_POINT_RED,3,3,{
	if(!is_party_leader()) end;
	instance_enable("Maristella Walter#ep21_rwi07",true);
	instance_enable("Yohan#ep21_rw10",true);
	instance_enable("Tan#ep21_rw08",true);
	instance_enable("Iana Operta#ep21_rwi07",true);
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "The main mast is completely broken・・.";
	npctalk "Maristella Walter : The main mast is completely broken・・.",instance_npcname("Maristella Walter#ep21_rwi07");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Ah, wait. I can see something・・ engraved here.";
	npctalk "Maristella Walter : Ah, wait. I can see something・・ engraved here.",instance_npcname("Maristella Walter#ep21_rwi07");
	next;
	cutin "ep21_iana01.png",1;
	mes "[Iana Operta]";
	mes "On the main mast? Interesting. Who would have done something so brave?";
	npctalk "Iana Operta : On the main mast? Interesting. Who would have done something so brave?",instance_npcname("Iana Operta#ep21_rwi07");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "・・I think it was me.";
	npctalk "Maristella Walter : ・・I think it was me.",instance_npcname("Maristella Walter#ep21_rwi07");
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "I didn't hear you clearly.";
	npctalk "Tan : I didn't hear you clearly.",instance_npcname("Tan#ep21_rw08");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Well, I said・・ that it was me・・.";
	npctalk "Maristella Walter : Well, I said・・ that it was me・・.",instance_npcname("Maristella Walter#ep21_rwi07");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "When they gave me permission to climb the main mast for the first time, I was so excited that I engraved my initials on it.";
	npctalk "Maristella Walter : When they gave me permission to climb the main mast for the first time, I was so excited that I engraved my initials on it.",instance_npcname("Maristella Walter#ep21_rwi07");
	next;
	cutin "ep21_iana04.png",1;
	mes "[Iana Operta]";
	mes "Wow・・.";
	npctalk "Iana Operta : Wow・・.",instance_npcname("Iana Operta#ep21_rwi07");
	next;
	cutin "ep21_tan02.png",1;
	mes "[Tan]";
	mes "Oh, I see・・ Ahem. Well, even the captain was young once.";
	npctalk "Tan : Oh, I see・・ Ahem. Well, even the captain was young once.",instance_npcname("Tan#ep21_rw08");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "・・Yes. I got into trouble after they caught me. I couldn't erase the marks that I had already made though.";
	npctalk "Maristella Walter : ・・Yes. I got into trouble after they caught me. I couldn't erase the marks that I had already made though.",instance_npcname("Maristella Walter#ep21_rwi07");
	next;
	cutin "ep21_yohan02.png",2;
	mes "[Yohan]";
	mes "Uhm, captain・・ did you really did that・・?";
	npctalk "Yohan : Uhm, captain・・ did you really did that・・?",instance_npcname("Yohan#ep21_rw10");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "・・There was a time like that・・. Anyway, if these are really the marks from back then, this ship is really the・・.";
	npctalk "Maristella Walter : ・・There was a time like that・・. Anyway, if these are really the marks from back then, this ship is really the・・.",instance_npcname("Maristella Walter#ep21_rwi07");
	instance_enable("Mast Traces#ep21_rw02",false);
	instance_enable("Maristella Walter#ep21_rwi07",false);
	instance_enable("Yohan#ep21_rw10",false);
	instance_enable("Tan#ep21_rw08",false);
	instance_enable("Iana Operta#ep21_rwi07",false);
	set_instance_var("mps", get_instance_var("mps") + 1);
	close3;
	
OnTouch:
	npctalk "!!!";
end;
}

1@wtgs,116,326,3	script(DISABLED)	Maristella Walter#ep21_rwi07	4_EP21_MARISTELLA,{
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "I mean・・ there was a time where I did what I want. It's an old story・・.";
	close3;
}

1@wtgs,114,328,3	script(DISABLED)	Yohan#ep21_rw10	4_EP21_YOHAN,{
	cutin "ep21_yohan02.png",2;
	mes "[Yohan]";
	mes "Captain・・ Hm, I could have never imagined・・ that she'd do something like that.";
	close3;
}

1@wtgs,110,327,5	script(DISABLED)	Tan#ep21_rw08	4_EP21_TAN,{
	cutin "ep21_tan02.png",1;
	mes "[Tan]";
	mes "Ahem. I should be careful with my words.";
	close3;
}

1@wtgs,109,326,5	script(DISABLED)	Iana Operta#ep21_rwi07	4_EP21_IANA,{
	cutin "ep21_iana04.png",1;
	mes "[Iana Operta]";
	mes "Ahehe, that's why I always say that Mari is cute, isn't she?";
	close3;
}

//= Event 3
1@wtgs,142,314,3	script(DISABLED)	Old Box#ep21_rw03	4_WOODBOX,3,3,{
	if(!is_party_leader()) end;
	instance_enable("Maristella Walter#ep21_rwi08",true);
	instance_enable("Yohan#ep21_rw11",true);
	instance_enable("Tan#ep21_rw09",true);
	instance_enable("Iana Operta#ep21_rwi08",true);
	cutin "ep21_yohan02.png",2;
	mes "[Yohan]";
	mes "・・Ouch! Ah, I almost fell down. Huh? There's a box under the plank.";
	npctalk "Yohan : ・・Ouch! Ah, I almost fell down. Huh? There's a box under the plank.",instance_npcname("Yohan#ep21_rw11");
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "Keep your eyes on the way, kid.";
	npctalk "Tan : Keep your eyes on the way, kid.",instance_npcname("Tan#ep21_rw09");
	next;
	cutin "ep21_yohan02.png",2;
	mes "[Yohan]";
	mes "You wouldn't normally think that・・ there'd be a hidden box in a place like this. Huh? These are・・.";
	npctalk "Yohan : You wouldn't normally think・・ that there'd be a hidden box in a place like this. Huh? These are・・.",instance_npcname("Yohan#ep21_rw11");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "What's the matter? Yohan.";
	npctalk "Maristella Walter : What's the matter? Yohan.",instance_npcname("Maristella Walter#ep21_rwi08");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "This box is full of・・ carved wooden toys.";
	npctalk "Yohan : This box is full of・・ carved wooden toys.",instance_npcname("Yohan#ep21_rw11");
	next;
	mes "[Yohan]";
	mes "Half of it have already decayed and rotted. Look, this one is shaped like a duck・・.";
	npctalk "Yohan : Half of it have already decayed and rotted. Look, this one is shaped like a duck・・.",instance_npcname("Yohan#ep21_rw11");
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "Does it matter.";
	npctalk "Tan : Does it matter.",instance_npcname("Tan#ep21_rw09");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "Carving animal toys from leftover wood・・, is captain Howell's hobby.";
	npctalk "Yohan : Carving animal toys from leftover wood・・, is captain Howell's hobby.",instance_npcname("Yohan#ep21_rw11");
	next;
	mes "[Yohan]";
	mes "He especially loved carving birds. Seagulls, and so on, Ahaha. This one is a bear with wings・・ It's not even a bird.";
	npctalk "Yohan : He especially loved carving birds. Seagulls, and so on, Ahaha. This one is a bear with wings・・ It's not even a bird.",instance_npcname("Yohan#ep21_rw11");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Ah. ・・right. He was saving it to give it to his daughter.";
	npctalk "Maristella Walter : Ah. ・・right. He was saving it to give it to his daughter.",instance_npcname("Maristella Walter#ep21_rwi08");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "・・・・.";
	npctalk "Yohan : ・・・・.",instance_npcname("Yohan#ep21_rw11");
	instance_enable("Old Box#ep21_rw03",false);
	instance_enable("Maristella Walter#ep21_rwi08",false);
	instance_enable("Yohan#ep21_rw11",false);
	instance_enable("Tan#ep21_rw09",false);
	instance_enable("Iana Operta#ep21_rwi08",false);
	set_instance_var("mps", get_instance_var("mps") + 1);
	close3;
	
OnTouch:
	if(is_party_leader())
		npctalk "!!!";
end;
}

1@wtgs,138,313,5	script(DISABLED)	Yohan#ep21_rw11	4_EP21_YOHAN,{
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "I shouldn't・・ have asked for one. I lost it while escaping from the ship.";
	close3;
}

1@wtgs,139,316,5	script(DISABLED)	Tan#ep21_rw09	4_EP21_TAN,{
	cutin "ep21_tan02.png",1;
	mes "[Tan]";
	mes "Why, don't look at me like that.";
	close3;
}

1@wtgs,142,316,3	script(DISABLED)	Maristella Walter#ep21_rwi08	4_EP21_MARISTELLA,{
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "・・I didn't know he made so many of them.";
	close3;
}

1@wtgs,141,312,7	script(DISABLED)	Iana Operta#ep21_rwi08	4_EP21_IANA,{
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "These reminds me of how I deliberately leave pieces of wood on the deck of the Walter ship.";
	close3;
}

//= Event 4
1@wtgs,66,37,3	script(DISABLED)	Crumpled Documents#ep21_rw	4_EP18_PAPERS,3,3,{
	if(!is_party_leader()) end;
	instance_enable("Maristella Walter#ep21_rwi09",true);
	instance_enable("Yohan#ep21_rw12",true);
	instance_enable("Tan#ep21_rw10",true);
	instance_enable("Iana Operta#ep21_rwi09",true);
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Oh, what's all of this?";
	npctalk "Iana Operta : Oh, what's all of this?",instance_npcname("Iana Operta#ep21_rwi09");
	next;
	cutin "ep21_iana01.png",1;
	mes "[Iana Operta]";
	mes "I see, I see. It looks like the supply order form. Let's see... the things written is almost unreadable because of the water's smudges.";
	npctalk "Iana Operta : I see, I see. It looks like the supply order form. Let's see... the things written is almost unreadable because of the water's smudges.",instance_npcname("Iana Operta#ep21_rwi09");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "But the final signature is recognizable. ・・It's a familiar mark.";
	npctalk "Maristella Walter : But the final signature is recognizable. ・・It's a familiar mark.",instance_npcname("Maristella Walter#ep21_rwi09");
	next;
	mes "[Maristella Walter]";
	mes "・・It's the captain's handwriting. and・・ name.";
	npctalk "Maristella Walter : ・・It's the captain's handwriting. and・・ 戚硯戚壱.",instance_npcname("Maristella Walter#ep21_rwi09");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "We can't be sure・・. the signature is smudged by the seawater, I can't see it properly.";
	npctalk "Yohan : We can't be sure・・. the signature is smudged by the seawater, I can't see it properly.",instance_npcname("Yohan#ep21_rw12");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Yohan, you are the one tasked to climb the top of the mast to be our rear scout, you should have no problem seeing it with your good eyesight,.";
	npctalk "Maristella Walter : Yohan, you are the one tasked to climb the top of the mast to be our rear scout, you should have no problem seeing it with your good eyesight.",instance_npcname("Maristella Walter#ep21_rwi09");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "I don't know・・ I really can't see it.";
	npctalk "Yohan : I don't know・・ I really can't see it.",instance_npcname("Yohan#ep21_rw12");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Alright. Let's just go with that. For now・・ I'll give you some time.";
	npctalk "Maristella Walter : Alright. Let's go just with that. For now・・ I'll give you some time.",instance_npcname("Maristella Walter#ep21_rwi09");
	instance_enable("Crumpled Documents#ep21_rw",false);
	instance_enable("Maristella Walter#ep21_rwi09",false);
	instance_enable("Yohan#ep21_rw12",false);
	instance_enable("Tan#ep21_rw10",false);
	instance_enable("Iana Operta#ep21_rwi09",false);
	set_instance_var("mps", get_instance_var("mps") + 1);
	close3;
	
OnTouch:
	if(is_party_leader())
		npctalk "!!!";
end;
}

1@wtgs,65,35,7	script(DISABLED)	Tan#ep21_rw10	4_EP21_TAN,{
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "・・I'm also familiar with the handwriting and the signature of captain Walter.";
	close3;
}

1@wtgs,63,37,7	script(DISABLED)	Maristella Walter#ep21_rwi09	4_EP21_MARISTELLA,{
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "That Yohan, there are times when being stubborn is not useful.";
	close3;
}

1@wtgs,65,39,5	script(DISABLED)	Yohan#ep21_rw12	4_EP21_YOHAN,{
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "I said・・ I can't see it, I truly don't・・.";
	close3;
}

1@wtgs,68,34,1	script(DISABLED)	Iana Operta#ep21_rwi09	4_EP21_IANA,{
	cutin "ep21_iana01.png",1;
	mes "[Iana Operta]";
	mes "Well, well, maybe it's possible not to see things?";
	close3;
}

//= Event 5
1@wtgs,184,128,3	script(DISABLED)	Rusty Fountain Pen#ep21_rw05	4_POINT_RED,3,3,{
	if(!is_party_leader()) end;
	instance_enable("Maristella Walter#ep21_rwi10",true);
	instance_enable("Yohan#ep21_rw13",true);
	instance_enable("Tan#ep21_rw11",true);
	instance_enable("Iana Operta#ep21_rwi10",true);
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "There's something on the floor here.";
	npctalk "Tan : There's something on the floor here.",instance_npcname("Tan#ep21_rw11");
	next;
	mes "[Tan]";
	mes "It's a fountain pen. It's already rusted, but it looks to luxurious to be lying around here・・.";
	npctalk "Tan : It's a fountain pen. It's already rusted, but it looks to luxurious to be lying around here・・.",instance_npcname("Tan#ep21_rw11");
	next;
	cutin "ep21_yohan02.png",2;
	mes "[Yohan]";
	mes "・・That is・・!";
	npctalk "Yohan : ・・That is・・!",instance_npcname("Yohan#ep21_rw13");
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "Why, do you recognize it??";
	npctalk "Tan : Why, do you recognize it??",instance_npcname("Tan#ep21_rw11");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "It looks like・・ the fountain pen that Mariposa treasured.";
	npctalk "Yohan : It looks like・・ the fountain pen that Mariposa treasured.",instance_npcname("Yohan#ep21_rw13");
	next;
	mes "[Yohan]";
	mes "It looks similar. But I can't really tell because of the rust・・.";
	npctalk "Yohan : It looks similar. But I can't really tell because of the rust・・.",instance_npcname("Yohan#ep21_rw13");
	next;
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "The Walter apprentice enginner, Mariposa?";
	npctalk "Iana Operta : The Walter apprentice enginner, Mariposa?",instance_npcname("Iana Operta#ep21_rwi10");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "She said it was a keepsake from her mother. Even though she knew the sea air wasn't good for it・・ she would bring it out and hold it once a day.";
	npctalk "Yohan : She said it was a keepsake from her mother. Even though she knew the sea air wasn't good for it・・ she would bring it out and hold it once a day.",instance_npcname("Yohan#ep21_rw13");
	next;
	mes "[Yohan]";
	mes "・・Could they have all died here? Or, are they still wandering・・ unable・・ to die?";
	npctalk "Yohan : ・・Could they have all died here? Or, are they still wandering・・ unable・・ to die?",instance_npcname("Yohan#ep21_rw13");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "I should have been here too・・. I'm a coward, I'm the only one who survived. Only me・・.";
	npctalk "Yohan : I should have been here too・・. I'm a coward, I'm the only one who survived. Only me・・.",instance_npcname("Yohan#ep21_rw13");
	next;
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Yohan.";
	npctalk "Iana Operta : Yohan.",instance_npcname("Iana Operta#ep21_rwi10");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Yohan, you are not a coward. The captain was the one who decided to have you escape. Are you saying that the captain is a coward?";
	npctalk "Maristella Walter : Yohan, you are not a coward. The captain was the one who decided to have you escape. Are you saying that the captain is a coward?",instance_npcname("Maristella Walter#ep21_rwi10");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "No, no・・ that's not what I meant・・.";
	npctalk "Yohan : No, no・・ that's not what I meant・・.",instance_npcname("Yohan#ep21_rw13");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Thanks to what you passed on, we were able to gain information on what happened and able to get some clue to track the Walter.";
	npctalk "Maristella Walter : Thanks to what you passed on, we were able to gain information on what happened and able to get some clue to track the Walter.",instance_npcname("Maristella Walter#ep21_rwi10");
	next;
	mes "[Maristella Walter]";
	mes "Regardless of everything what happened, surviving a storm and being alive is something to be grateful as a sailor.";
	npctalk "Maristella Walter : Regardless of everything what happened, surviving a storm and being alive is something to be grateful as a sailor.",instance_npcname("Maristella Walter#ep21_rwi10");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Yohan. You don't have to carry the guilt.";
	npctalk "Maristella Walter : Yohan. You don't have to carry the guilt.",instance_npcname("Maristella Walter#ep21_rwi10");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "・・.";
	npctalk "Yohan : ・・.",instance_npcname("Yohan#ep21_rw13");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "・・With this, " + strcharinfo(0) + ". I'm certain.";
	npctalk "Maristella Walter : ・・With this, I'm certain.",instance_npcname("Maristella Walter#ep21_rwi10");
	next;
	mes "[Maristella Walter]";
	mes "It's time to face reality. Let's head to the captain's quarter.";
	npctalk "Maristella Walter : It's time to face reality. Let's head to the captain's quarter.",instance_npcname("Maristella Walter#ep21_rwi10");
	instance_enable("Rusty Fountain Pen#ep21_rw05",false);
	instance_enable("Maristella Walter#ep21_rwi10",false);
	instance_enable("Yohan#ep21_rw13",false);
	instance_enable("Tan#ep21_rw11",false);
	instance_enable("Iana Operta#ep21_rwi10",false);
	set_instance_var("mps", get_instance_var("mps") + 1);
	close3;
}

1@wtgs,185,125,1	script(DISABLED)	Yohan#ep21_rw13	4_EP21_YOHAN,{
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "I am coward, I'm the only who survived・・!";
	close3;
}

1@wtgs,187,127,3	script(DISABLED)	Tan#ep21_rw11	4_EP21_TAN,{
	cutin "ep21_tan02.png",1;
	mes "[Tan]";
	mes "You don't need to take it like that・・.";
	close3;
}

1@wtgs,186,129,3	script(DISABLED)	Iana Operta#ep21_rwi10	4_EP21_IANA,{
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Yohan, it's okay.";
	close3;
}

1@wtgs,183,126,7	script(DISABLED)	Maristella Walter#ep21_rwi10	4_EP21_MARISTELLA,{
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "You don't need to carry that guilt, It's not your fault・・.";
	close3;
}

//= Event 6
1@wtgs,145,129,3	script(DISABLED)	Box Under a Bed#ep21_rw0	4_POINT_RED,3,3,{
	if(!is_party_leader()) end;
	cutin "ep21_iana01.png",1;
	instance_enable("Maristella Walter#ep21_rwi11",true);
	instance_enable("Yohan#ep21_rw14",true);
	instance_enable("Tan#ep21_rw12",true);
	instance_enable("Iana Operta#ep21_rwi11",true);
	mes "[Iana Operta]";
	mes "This box is still intact without wear nor damage.";
	npctalk "Iana Operta : This box is still intact without wear nor damage.",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "Should we open it? We don't know what might be inside.";
	npctalk "Iana Operta : Should we open it? We don't know what might be inside.",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "It would be better if Captain Walter makes the call.";
	npctalk "Tan : It would be better if Captain Walter makes the call.",instance_npcname("Tan#ep21_rw12");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Let's check if there's anything left inside.";
	npctalk "Maristella Walter : Let's check if there's anything left inside.",instance_npcname("Maristella Walter#ep21_rwi11");
	next;
	cutin "ep21_iana04.png",1;
	mes "[Iana Operta]";
	mes "Okay. Should I break the lock? Yah【 ・・Oh, it's a journal.";
	npctalk "Iana Operta : Okay. Should I break the lock? Yah【 ・・Oh, it's a journal.",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	cutin "ep21_iana01.png",1;
	mes "[Iana Operta]";
	mes "I'm sorry for the owner this journal. I'm going to read it. It's better if we start reading from the end.";
	npctalk "Iana Operta : I'm sorry for the owner this journal. I'm going to read it. It's better if we start reading from the end.",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "'We manage to let captain Maristella escape safely, but our ship was adrift in the open sea. What will happen now?'";
	npctalk "Iana Operta : 'We manage to let captain Maristella escape safely, but our ship was adrift in the open sea. What will happen now?'",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "'The sails is broken and we've been drifting aimlessly for days. But something incredible happened・・. A strange woman suddenly appeared on the deck.'";
	npctalk "Iana Operta : 'The sails is broken and we've been drifting aimlessly for days. But something incredible happened・・. A strange woman suddenly appeared on the deck.'",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "'Was she a witch? Or perhaps a god? On the adrift ship, she asked what the captain desired the most.'";
	npctalk "Iana Operta : 'What she a witch? Or perhaps a god? On the adrift ship, she asked what the captain desired the most.'",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "'When the captain answered, everyone and the ship ended up in this state・・.'";
	npctalk "Iana Operta : 'When the captain answered, everyone and the ship ended up in this state・・.'",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "'We didn't regret it. What the captain wished for was the same as what we wanted.'";
	npctalk "Iana Operta : 'We didn't regret it. What the captain wished for was the same as what we wanted.'",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "'But, I felt uneasy. My memories are fading bit by bit. Some of my comrades have completely turned into monsters.'";
	npctalk "Iana Operta : 'But, I felt uneasy. My memories are fading bit by bit. Some of my comrades have completely turned into monsters.'",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "'Even if I forget the fact that I left this record, I hope it remains intact and is passed on the someone・・.'";
	npctalk "Iana Operta : 'Even if I forget the fact that I left this record, I hope it remains intact and is passed on the someone・・.'",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	mes "[Iana Operta]";
	mes "・・That's the end.";
	npctalk "Iana Operta : ・・That's the end.",instance_npcname("Iana Operta#ep21_rwi11");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Who could be that strange woman?";
	npctalk "Maristella Walter : Who could be that strange woman?",instance_npcname("Maristella Walter#ep21_rwi11");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "A being that grants wish・・, nothing else but a witch.";
	npctalk "Yohan : A being that grants wish・・, nothing else but a witch.",instance_npcname("Yohan#ep21_rw14");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "I can't think of a wish that the captain made that turned the ship into this.";
	npctalk "Maristella Walter : I can't think of a wish that the captain made that turned the ship into this.",instance_npcname("Maristella Walter#ep21_rwi11");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "But we are certain now. We know what this ship is.";
	npctalk "Maristella Walter : But we are certain now. We know what this ship is.",instance_npcname("Maristella Walter#ep21_rwi11");
	instance_enable("Box Under a Bed#ep21_rw0",false);
	instance_enable("Maristella Walter#ep21_rwi11",false);
	instance_enable("Yohan#ep21_rw14",false);
	instance_enable("Tan#ep21_rw12",false);
	instance_enable("Iana Operta#ep21_rwi11",false);
	set_instance_var("mps", get_instance_var("mps") + 1);
	close3;
}

1@wtgs,147,124,1	script(DISABLED)	Iana Operta#ep21_rwi11	4_EP21_IANA,{
	cutin "ep21_iana03.png",1;
	mes "[Iana Operta]";
	mes "Huh? Why are you looking at me like that? The lock broke the moment I touched it because it was already corroded. It's not because I'm strong or something.";
	close3;
}

1@wtgs,149,126,1	script(DISABLED)	Yohan#ep21_rw14	4_EP21_YOHAN,{
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "If, there's something has caused them to drift like this forever, surely it's not a good thing・・ definitely not・・.";
	close3;
}

1@wtgs,149,129,3	script(DISABLED)	Maristella Walter#ep21_rwi11	4_EP21_MARISTELLA,{
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "I've heard of stories about a sea witch who grants wishes. Could it be that one? I'm too old to believe in things like that・・.";
	close3;
}

1@wtgs,147,130,3	script(DISABLED)	Tan#ep21_rw12	4_EP21_TAN,{
	cutin "ep21_tan02.png",1;
	mes "[Tan]";
	mes "Hm, It was a huge help that someone left behind a journal.";
	close3;
}

//= WARP
1@wtgs,122,318,0	script(DISABLED)	#ep21_rwarp01	WARPNPC,2,2,{
	end;
	
OnTouch:
	switch(atoi(replacestr(strnpcinfo(2),"ep21_rwarp0",""))){
		case 1:
			.@x = 39; 
			.@y = 30; 
			break;
			
		case 2:
			.@x = 118; 
			.@y = 318; 
			break;

		case 3:
			.@x = 211; 
			.@y = 110; 
			break;

		case 4:
			.@x = 56; 
			.@y = 30; 
			break;

		case  5:
			if(is_party_leader() && get_instance_var("daily") == true && get_instance_var("ds") == 1){
				instance_enable("Maristella Walter#ep21_rwi02",true);
				cutin "ep21_maristella01.png",1;
				mes "[Maristella Walter]";
				mes "We are here finally. On the captain's quarter.";
				npctalk "Maristella Walter : We are here finally. On the captain's quarter.";
				next;
				cutin "ep21_maristella03.png",1;
				mes "[Maristella Walter]";
				mes "・・Sigh. I'm fine. I can do this. I know that it's not the captain.";
				npctalk "Maristella Walter : ・・Sigh. I'm fine. I can do this. I know that it's not the captain.";
				next;
				cutin "ep21_maristella01.png",1;
				mes "[Maristella Walter]";
				mes "We also need to disperse the magic here. Let's go to the captain's quarter.";
				npctalk "Maristella Walter : We also need to disperse the magic here. Let's go to the captain's quarter.";
				close2;
				cutin "",255;
				instance_enable("Maristella Walter#ep21_rwi02",false);
				set_instance_var("ds",2);
				instance_event("#EP21_GS_control","OnBossSummon",false);
				.@x = 152;
				.@y = 16;
				break;
			}
			if(get_instance_var("ds") == 2){
				.@x = 152;
				.@y = 16;
				break;
			}
			if(get_instance_var("daily") == true) end;
			if(get_instance_var("mps") < 9 && !is_party_leader()) end;
			if(get_instance_var("mps") < 8){
				instance_enable("Maristella Walter#ep21_rwi02",true);
				cutin "ep21_maristella01.png",1;
				mes "[Maristella Walter]";
				mes "It seems there's still something we need to check. I'd like to search more before we proceed.";
				npctalk "Maristella Walter : It seems there's still something we need to check. I'd like to search more before we proceed.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				mes "[Maristella Walter]";
				mes "We should look around the ship a little more before heading inside the captain's quarter.";
				npctalk "Maristella Walter : We should look around the ship a little more before heading inside the captain's quarter.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				cutin "ep21_maristella03.png",1;
				mes "[Maristella Walter]";
				mes "I want to be as certain as possible・・ even if only a little bit.";
				npctalk "Maristella Walter : I want to be as certain as possible・・ even if only a little bit.",instance_npcname("Maristella Walter#ep21_rwi02");
				close2;
				cutin "",255;
				instance_enable("Maristella Walter#ep21_rwi02",false);
				end;
			}
			if(get_instance_var("mps") == 8){
				cutin "ep21_maristella01.png",1;
				instance_enable("Maristella Walter#ep21_rwi02",true);
				instance_enable("Yohan#ep21_rw05",true);
				instance_enable("Tan#ep21_rw03",true);
				instance_enable("Iana Operta#ep21_rwi02",true);
				mes "[Maristella Walter]";
				mes "It seems that we've found everything we could insde this ship. But now, we must accept the truth.";
				npctalk "Maristella Walter : It seems that we've found everything we could insde this ship. But now, we must accept the truth.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				cutin "ep21_maristella03.png",1;
				mes "[Maristella Walter]";
				mes "We should be ready. Yes , be ready mentally ・・and be ready to fight.";
				npctalk "Maristella Walter : We should be ready. Yes, be ready mentally ・・and be ready to fight.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				cutin "ep21_yohan03.png",2;
				mes "[Yohan]";
				mes "Captain・・.";
				npctalk "Yohan : Captain・・.",instance_npcname("Yohan#ep21_rw05");
				next;
				cutin "ep21_maristella01.png",1;
				mes "[Maristella Walter]";
				mes "I don't have any doubts that this ship is missing Walter. We still don't know how it ended up this state though.";
				npctalk "Maristella Walter : I don't have any doubts that this ship is missing Walter. We still don't know how it ended up this state though.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				mes "[Maristella Walter]";
				mes "Above this room is the captain's quarter. The person we'll encounter there surely・・.";
				npctalk "Maristella Walter : Above this room is the captain's quarter. The person we'll encounter there surely・・.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				cutin "ep21_maristella03.png",1;
				mes "[Maristella Walter]";
				mes "Surely.";
				npctalk "Maristella Walter : Surely.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				cutin "ep21_tan02.png",1;
				mes "[Tan]";
				mes "The captain's father. Former captain, Howell Megas Walter.";
				npctalk "Tan : The captain's father. Former captain, Howell Megas Walter.",instance_npcname("Tan#ep21_rw03");
				next;
				cutin "ep21_yohan03.png",2;
				mes "[Yohan]";
				mes "・・!!!!!";
				npctalk "Yohan : ・・!!!!!",instance_npcname("Yohan#ep21_rw05");
				next;
				cutin "ep21_maristella01.png",1;
				mes "[Maristella Walter]";
				mes "Yes. Just like the undead sailors, it's likely that the captain won't recognize us.";
				npctalk "Maristella Walter : Yes. Just like the undead sailors, it's likely that the captain won't recognize us.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				mes "[Maristella Walter]";
				mes "We'll have to fight.";
				npctalk "Maristella Walter : We'll have to fight.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				cutin "ep21_iana01.png",1;
				mes "[Iana Operta]";
				mes "We don't know that yet. According to the journal, there were some who retained onto their sanity.";
				npctalk "Iana Operta : We don't know that yet. According to the journal, there were some who retained onto their sanity.",instance_npcname("Iana Operta#ep21_rwi02");
				next;
				cutin "ep21_iana02.png",1;
				mes "[Iana Operta]";
				mes "Let's go, to the captain's quarter. Let's see it for ourselves.";
				npctalk "Iana Operta : Let's go, to the captain's quarter. Let's see it for ourselves.",instance_npcname("Iana Operta#ep21_rwi02");
				next;
				cutin "ep21_maristella01.png",1;
				mes "[Maristella Walter]";
				mes "Yes. There are also other things we need to confirm.";
				npctalk "Maristella Walter : Yes. There are also other things we need to confirm.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				mes "[Maristella Walter]";
				mes "The identity of the 'strange woman' that was mentioned in the journal. And what the captain wished for・・.";
				npctalk "Maristella Walter : The identity of the 'strange woman' that was mentioned in the journal. What the captain wished for・・.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				mes "[Maristella Walter]";
				mes "Why the walter continued to wander the sea and attack the cult's ship even after becoming like this.";
				npctalk "Maristella Walter : Why the walter continued to wander the sea and attack the cult's ship even after becoming like this.",instance_npcname("Maristella Walter#ep21_rwi02");
				next;
				mes "[Maristella Walter]";
				mes "All the answer lies within the captain's quarter. Let's go now・・ to the captain's quarter.";
				npctalk "Maristella Walter : All the answer lies within the captain's quarter. Let's go now・・ to the captain's quarter.",instance_npcname("Maristella Walter#ep21_rwi02");
				close2;
				instance_event("#EP21_GS_control","OnEvent",false);
			}
			.@x = 152;
			.@y = 16;
			break;

		case 6:
			.@x = 272;
			.@y = 315;
			break;

		case 7:
			warp "jor_albe",191,182;
			end;
	}
	warp get_instance_var("map$"),.@x,.@y;
end;
}

1@wtgs,35,30,0	duplicate(#ep21_rwarp01)	#ep21_rwarp02	WARPNPC,2,2
1@wtgs,60,30,0	duplicate(#ep21_rwarp01)	#ep21_rwarp03	WARPNPC,2,2
1@wtgs,215,110,0	duplicate(#ep21_rwarp01)	#ep21_rwarp04	WARPNPC,2,2
1@wtgs,105,110,0	duplicate(#ep21_rwarp01)	#ep21_rwarp05	WARPNPC,2,2
1@wtgs,336,26,0	duplicate(#ep21_rwarp01)	#ep21_rwarp06	WARPNPC,2,2
1@wtgs,283,306,0	duplicate(#ep21_rwarp01)	#ep21_rwarp07	WARPNPC,2,2

1@wtgs,109,111,3	script(DISABLED)	Iana Operta#ep21_rwi02	4_EP21_IANA,{
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Let's go. Everyone, stay sharp.";
	close3;
}

1@wtgs,104,113,3	script(DISABLED)	Yohan#ep21_rw05	4_EP21_YOHAN,{
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "・・I don't want to. I don't want to see it.";
	close3;
}

1@wtgs,105,115,3	script(DISABLED)	Tan#ep21_rw03	4_EP21_TAN,{
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "・・It's been a long journey. Everyone, you did well.";
	close3;
}

1@wtgs,107,113,3	script(DISABLED)	Maristella Walter#ep21_rwi02	4_EP21_MARISTELLA,{
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Let's go to the captain's quarter. Whatever awaits, I'm ready to face it.";
	close3;
}

1@wtgs,170,25,3	script(DISABLED)	Ghostship Captain#ep21_rw01	4_EP21_HOWELL_S,{
	if(!is_party_leader() || get_instance_var("mps") < 9) end;
	instance_enable("Maristella Walter#ep21_rwi03",true);
	instance_enable("Yohan#ep21_rw06",true);
	instance_enable("Tan#ep21_rw04",true);
	instance_enable("Iana Operta#ep21_rwi03",true);
	cutin "ep21_howell_u02.png",0;
	mes "[Ghostship Captain]";
	mes "・・Grrrr・・r.";
	npctalk "Ghostship Captain : ・・Grrrr・・r.";
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Ah・・!";
	npctalk "Maristella Walter : Ah・・!",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "C-captain・・! It's me, Yohan・・! Captain!";
	npctalk "Yohan : C-captain・・! It's me, Yohan・・! Captain!",instance_npcname("Yohan#ep21_rw06");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "It seems like he doesn't recognize us. just like the others・・.";
	npctalk "Maristella Walter : It seems like he doesn't recognize us. just like the others・・.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Isn't there anything we can do? The captain is in a state like this・・ Is there really nothing we can do!";
	npctalk "Yohan : Isn't there anything we can do? The captain is in a state like this・・ Is there really nothing we can do!",instance_npcname("Yohan#ep21_rw06");
	next;
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "We won't do anything. But there is something we must do, Yohan.";
	npctalk "Iana Operta : We won't do anything. But there is something we must do, Yohan.",instance_npcname("Iana Operta#ep21_rwi03");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Yes. Just like with the others・・. We have to fight, Yohan.";
	npctalk "Maristella Walter : Yes. Just like with the others・・. We have to fight, Yohan.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "I, I can't・・, I can't do it. Even at this state, I can't point my sword to the captain. To me he is still the captain!";
	npctalk "Yohan : I, I can't・・, I can't do it. Even at this state, I can't point my sword to the captain. To me he is still the captain!",instance_npcname("Yohan#ep21_rw06");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Yohan, you should have prepared yourself for this when you boarded the ship.";
	npctalk "Maristella Walter : Yohan, you should have prepared yourself for this when you boarded the ship.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	mes "[Maristella Walter]";
	mes "If you find it difficult, step aside.";
	npctalk "Maristella Walter : If you find it difficult, step aside.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Captain, captain・・ He's your father!";
	npctalk "Yohan : Captain, captain・・ He's your father!",instance_npcname("Yohan#ep21_rw06");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "I know! I know. You're not the only one suffering right now, Yohan.";
	npctalk "Maristella Walter : I know! I know. You're not the only one suffering right now, Yohan.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_howell_u02.png",0;
	mes "[Ghostship Captain]";
	mes "・・Gr・・Grr・・.";
	npctalk "Ghostship Captain : ・・Gr・・Grr・・.";
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Even without exchanging words, uncovering the truth, knowing he's my father the more I need to raise my weapon!";
	npctalk "Maristella Walter : Even without exchanging words, uncovering the truth, knowing he's my father the more I need to raise my weapon!",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "We all loved the captain in our own way. It's only natural for it to be painful.";
	npctalk "Maristella Walter : We all loved the captain in our own way. It's only natural for it to be painful.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Ugh・・, captain. I'm, sorry.";
	npctalk "Yohan : Ugh・・, captain. I'm, sorry.",instance_npcname("Yohan#ep21_rw06");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Iana, take care of Yohan. It seems that he won't be able to fight.";
	npctalk "Maristella Walter : Iana, take care of Yohan. It seems that he won't be able to fight.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Alright, don't worry.";
	npctalk "Iana Operta : Alright, don't worry.",instance_npcname("Iana Operta#ep21_rwi03");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Then " + strcharinfo(0) + ", let's prepare for battle. Attack as soon as I give the signal.";
	npctalk "Maristella Walter : Then, let's prepare for battle. Attack as soon as I give the signal.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "・・Now!";
	npctalk "Maristella Walter : ・・Now!",instance_npcname("Maristella Walter#ep21_rwi03");
	close2;
	cutin "",255;
	instance_enable("Ghostship Captain#ep21_rw01",false);
	instance_enable("Maristella Walter#ep21_rwi03",false);
	instance_enable("Yohan#ep21_rw06",false);
	instance_enable("Tan#ep21_rw04",false);
	instance_enable("Iana Operta#ep21_rwi03",false);
	instance_event("#EP21_GS_control","OnBossSummon",false);
	end;	
}

1@wtgs,170,25,3	script(DISABLED)	Ghostship Captain#ep21_rw02	4_EP21_HOWELL_S,{
	if(!is_party_leader() || get_instance_var("mps") < 10) end;
	instance_enable("Maristella Walter#ep21_rwi03",true);
	instance_enable("Yohan#ep21_rw06",true);
	instance_enable("Tan#ep21_rw04",true);
	instance_enable("Iana Operta#ep21_rwi03",true);
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Now, the final blow・・!";
	npctalk "Maristella Walter : Now, the final blow・・!",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	mes "[Maristella Walter]";
	mes "Please, " + strcharinfo(0) + ". Please, end it please・・!";
	npctalk "Maristella Walter : Please. Please, end it please・・!",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_howell_u02.png",0;
	mes "[Ghostship Captain]";
	mes "・・Gr, Ma・・. ri・・st, ella.";
	npctalk "Ghostship Captain : ・・Gr, Ma・・. ri・・st, ella.";
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Wait, did he just said, something? " + strcharinfo(0) + "! Wait a moment!";
	npctalk "Maristella Walter : Wait, did he just said, something? Wait a moment!",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_howell_u02.png",0;
	mes "[Ghostship Captain]";
	mes "Mari, stella・・, you・・ survived.";
	npctalk "Ghostship Captain : Mari, stella・・, you・・ survived.";
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Father!";
	npctalk "Maristella Walter : Father!",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "・・Oh, is this even possible? I thought the captain already lost his consciousness and turned into a monster.";
	npctalk "Tan : ・・Oh, is this even possible? I thought the captain already lost his consciousness and turned into a monster.",instance_npcname("Tan#ep21_rw04");
	next;
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Those actions, gestures・・ They all seem like the captains. Even if his appearance is different from our memories・・ It's definitely him.";
	npctalk "Iana Operta : Those actions, gestures・・ They all seem like the captains. Even if his appearance is different from our memories・・ It's definitely him.",instance_npcname("Iana Operta#ep21_rwi03");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Captain! Yo-you've come back! It's Yohan! It's me!";
	npctalk "Yohan : Captain! Yo-you've come back! It's Yohan! It's me!",instance_npcname("Yohan#ep21_rw06");
	next;
	cutin "ep21_howell_u02.png",0;
	mes "[Ghostship Captain]";
	mes "Yo, Yohan・・ I, am・・ glad. You are alive・・.";
	npctalk "Ghostship Captain : Yo, Yohan・・ I, am・・ glad. You are alive・・.";
	next;
	mes "[Ghostship Captain]";
	mes "Al, also, Ia-na・・ You're, here too・・. It, really came true・・ Haha.";
	npctalk "Ghostship Captain : Al, also, Ia-na・・ You're, here too・・. It, really came true・・ Haha.";
	next;
	setnpcdisplay(instance_npcname("Ghostship Captain#ep21_rw02"),4_EP21_HOWELL);
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "Everything I wished for in  life has came true. I have no regrets.";
	npctalk "Howell Megas Walter : Everything I wished for in  life has came true. I have no regrets.";
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Oh, father. All of this. What happened?";
	npctalk "Maristella Walter : Oh, father. All of this. What happened?",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	mes "[Maristella Walter]";
	mes "Why is the ship like this, why did everyone end up like that・・.";
	npctalk "Maristella Walter : Why is the ship like this, why did everyone end up like that・・.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "It's a long story.";
	npctalk "Howell Megas Walter : It's a long story.";
	next;
	mes "[Howell Megas Walter]";
	mes "On the adrift ship, a being appeared as death approached, perhaps a god, perhaps a devil.";
	npctalk "Howell Megas Walter : On the adrift ship, a being appeared as death approached, perhaps a god, perhaps a devil.";
	next;
	mes "[Howell Megas Walter]";
	mes "The crew and I only had one wish.";
	npctalk "Howell Megas Walter : The crew and I only had one wish.";
	next;
	mes "[Howell Megas Walter]";
	mes "<B>Even in death, we wish to guard the seas where the Walter fleet sails. We wanted to sink every cult ships that would stand in your way.</B>";
	npctalk "Howell Megas Walter : Even in death, we wish to guard the seas where the Walter fleet sails. We wanted to sink every cult ships that would stand in your way.";
	next;
	mes "[Howell Megas Walter]";
	mes "But it seems that time has come to an end.";
	npctalk "Howell Megas Walter : But it seems that time has come to an end.";
	next;
	setnpcdisplay(instance_npcname("Ghostship Captain#ep21_rw02"),4_EP21_HOWELL_S);
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Captain・・.";
	npctalk "Maristella Walter : Captain・・.",instance_npcname("Maristella Walter#ep21_rwi03");
	next;
	setnpcdisplay(instance_npcname("Ghostship Captain#ep21_rw02"),4_EP21_HOWELL);
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "I can feel the power that is keeping us here fading.";
	npctalk "Howell Megas Walter : I can feel the power that is keeping us here fading.";
	next;
	mes "[Howell Megas Walter]";
	mes "Maybe the power is weakening, I can even shed my undead form and let you see me, even if only for a moment.";
	npctalk "Howell Megas Walter : Maybe the power is weakening, I can even shed my undead form and let you see me, even if only for a moment.";
	next;
	mes "[Howell Megas Walter]";
	mes "The moon is rising・・. Perhaps this is the last moon that I will see myself.";
	npctalk "Howell Megas Walter : The moon is rising・・. Perhaps this is the last moon that I will see myself.";
	next;
	mes "[Howell Megas Walter]";
	mes "It might be a mere chance that you're here at this moment, but I'm glad for it, Maristella.";
	npctalk "Howell Megas Walter : It might be a mere chance that you're here at this moment, but I'm glad for it, Maristella.";
	close2;
	cutin "",255;
	instance_event("#EP21_GS_control","OnEvent",false);
	end;
}

1@wtgs,165,27,5	script(DISABLED)	Maristella Walter#ep21_rwi03	4_EP21_MARISTELLA,{
	if(isbegin_quest(16821) == 1){
		cutin "ep21_maristella01.png",1;
		mes "[Maristella Walter]";
		mes "After fighting, I became more certain. That was not the captain.";
		next;
		mes "[Maristella Walter]";
		mes "It may look like the captain, but it's an entirely mindless being. Surely what we talked about before hasn't happened yet.";
		next;
		mes "[Maristella Walter]";
		mes "I'm starting to feel uneasy. Let's hurry and go outside.";
		next;
		mes "[Maristella Walter]";
		mes "Oh, wait. I think I heard something from the outside. The door・・ it's opening.";
		completequest 16821;
		setquest 16822;
		setquest 16823;
		if(isbegin_quest(16825) == 1){
			completequest 16825;
			getitem "EP21_Wigner_Ticket",10;
			add_reputation_points(13,5);
		}
		close2;
		cutin "",255;
		warp get_instance_var("map$"),309,25;
		end;
	}
	if(isbegin_quest(16826) == 1 || isbegin_quest(16824) == 1){
		mes "[Maristella Walter]";
		mes "Sigh, It's finally over. Let's head out. To be honest, I feel a bit uneasy.";
		next;
		mes "[Maristella Walter]";
		mes "But this is something that we must continue to do, I don't intend to avoid it.";
		next;
		mes "[Maristella Walter]";
		mes "Shall we go outside. I heard something from that side. It's probably a door opening.";
		next;
		mes "[Maristella Walter]";
		mes "Since today's work is done. Please rest up, "+strcharinfo(0)+", Thank you for today.";
		if(isbegin_quest(16824) == 1)
			erasequest 16824;
		if(isbegin_quest(16826) == 1)
			erasequest 16826;
		getitem "EP21_Wigner_Ticket",10;
		add_reputation_points(13,5);
		close;
	}
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "We are connected, you will always remain in my heart, father. Always.";
	close3;
}

1@wtgs,164,23,5	script(DISABLED)	Yohan#ep21_rw06	4_EP21_YOHAN,{
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Ugh, I can't let the captain see me like this.";
	close3;
}

1@wtgs,168,23,7	script(DISABLED)	Iana Operta#ep21_rwi03	4_EP21_IANA,{
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Alright, let's do what we must do.";
	close3;
}

1@wtgs,166,21,7	script(DISABLED)	Tan#ep21_rw04	4_EP21_TAN,{
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "We might have more mysteries to solve, but let's leave that for later.";
	close3;
}

1@wtgs,312,25,3	script(DISABLED)	Howell Megas Walter#ep21_r	4_EP21_HOWELL,{
	if(!is_party_leader() || get_instance_var("mps") < 11) end;
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "I'm one of the dead here, my path differs from yours. It seems my time is coming to an end.";
	npctalk "Howell Megas Walter : I'm one of the dead here, my path differs from yours. It seems my time is coming to an end.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Is it because we came here? Did we cause this by coming?";
	npctalk "Yohan : Is it because we came here? Did we cause this by coming?",instance_npcname("Yohan#ep21_rw07");
	next;
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "Don't worry, Yohan. It's not because of you coming here. I was always worried about you because you worry too much, you little rascal.";
	npctalk "Howell Megas Walter : Don't worry, Yohan. It's not because of you coming here. I was always worried about you because you worry too much, you little rascal.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "The mana that is sustaining this ship is running out.";
	npctalk "Howell Megas Walter : The mana that is sustaining this ship is running out.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "If the ship is being sustained by some form of mana, our presence and actions here might have hastened it・・.";
	npctalk "Maristella Walter : If the ship is being sustained by some form of mana, our presence and actions here might have hastened it・・.",instance_npcname("Maristella Walter#ep21_rw");
	next;
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "It's alright. Even if what you say is true. Maristella, trust your judgement more.";
	npctalk "Howell Megas Walter : It's alright. Even if what you say is true. Maristella, trust your judgement more.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "If the time simply passed by without meeting you, maybe I might have lingered a bit more, perhaps sinking more cult ship.";
	npctalk "Howell Megas Walter : If the time simply passed by without meeting you, maybe I might have lingered a bit more, perhaps sinking more cult ship.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "But after that, I might have continued to be a monster and attack every ship in sight without a reason.";
	npctalk "Howell Megas Walter : But after that, I might have continued to be a monster and attack every ship in sight without a reason.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "The mana ran out before that ever happened, and I was able to meet you. I consider it a blessing.";
	npctalk "Howell Megas Walter : The mana ran out before that ever happened, and I was able to meet you. I consider it a blessing.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "Indeed. " + strcharinfo(0) + ", It's also a blessing that you have good comrades.";
	npctalk "Howell Megas Walter : Indeed. It's also a blessing that you have good comrades.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "Give my regards to Epesto as well. I suppose he wasn't able to come here because he was busy protecting your crew, Maristelle.";
	npctalk "Howell Megas Walter : Give my regards to Epesto as well. I suppose he wasn't able to come here because he was busy protecting your crew, Maristelle.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "With such good people by your side, it's reassures me that you'll do just fine without me by your side.";
	npctalk "Howell Megas Walter : With such good people by your side, it's reassures me that you'll do just fine without me by your side.",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	cutin "ep21_maristella03.png",1;
	mes "[Maristella Walter]";
	mes "Sob・・.";
	npctalk "Maristella Walter : Sob・・.",instance_npcname("Maristella Walter#ep21_rw");
	next;
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "Don't make that face. Now! I'd like to spend my final moments alone. I could order you to leave as the captain of the Walter!";
	npctalk "Howell Megas Walter : Don't make that face. Now! I'd like to spend my final moments alone. I could order you to leave as the captain of the Walter!",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	mes "[Howell Megas Walter]";
	mes "But I'll make it a request instead. The exit is at three o'clock. Will you clear the space for me?";
	npctalk "Howell Megas Walter : But I'll make it a request instead. The exit is at three o'clock. Will you clear the space for me?",instance_npcname("Howell Megas Walter#ep21_r");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "・・Yes, as a Walter, and as Maristella. I will do as you ask. I understand, I know your heart well・・!";
	npctalk "Maristella Walter : ・・Yes, as a Walter, and as Maristella. I will do as you ask. I understand, I know your heart well・・!",instance_npcname("Maristella Walter#ep21_rw");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "I've witnessed and understood how you protected this ship. Now, I will carry out that duty with my life!";
	npctalk "Maristella Walter : I've witnessed and understood how you protected this ship. Now, I will carry out that duty with my life!",instance_npcname("Maristella Walter#ep21_rw");
	next;
	mes "[Maristella Walter]";
	mes "I won't worry you! So, please.";
	npctalk "Maristella Walter : I won't worry you! So, please.",instance_npcname("Maristella Walter#ep21_rw");
	next;
	mes "[Maristella Walter]";
	mes "Rest in peace.";
	npctalk "Maristella Walter : Rest in peace.",instance_npcname("Maristella Walter#ep21_rw");
	next;
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Leave the Walter fleet to us!";
	npctalk "Yohan : Leave the Walter fleet to us!",instance_npcname("Yohan#ep21_rw07");
	next;
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "I'll make sure to reign them tight so they don't go charging recklessly, Captain, so don't worry.";
	npctalk "Iana Operta : I'll make sure to reign them tight so they don't go charging recklessly, Captain, so don't worry.",instance_npcname("Iana Operta#ep21_rw");
	next;
	cutin "ep21_howell01.png",0;
	mes "[Howell Megas Walter]";
	mes "That's reassuring. Then. Take good care of the Walter. All of you・・.";
	npctalk "Howell Megas Walter : That's reassuring. Then. Take good care of the Walter. All of you・・.",instance_npcname("Howell Megas Walter#ep21_r");
	close2;
	cutin "",255;
	instance_event("#EP21_GS_control","OnEvent",false);
	end;
}

1@wtgs,308,21,7	script(DISABLED)	Tan#ep21_rw05	4_EP21_TAN,{
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "We might have more mysteries to solve, but let's leave that for later.";
	close3;
}

1@wtgs,306,23,5	script(DISABLED)	Yohan#ep21_rw07	4_EP21_YOHAN,{
	cutin "ep21_yohan03.png",2;
	mes "[Yohan]";
	mes "Ugh, I can't let the captain see me like this.";
	close3;
}

1@wtgs,307,27,5	script(DISABLED)	Maristella Walter#ep21_rwi04	4_EP21_MARISTELLA,{
	if(get_instance_var("daily") != true){
		cutin "ep21_maristella01.png",1;
		mes "[Maristella Walter]";
		mes "We are connected, you will always remain in my heart, father. Always.";
		close3;
	} else
		npctalk "Maristella Walter : Let's head out.";
	end;
}

1@wtgs,310,23,7	script(DISABLED)	Iana Operta#ep21_rwi04	4_EP21_IANA,{
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Alright, let's do what we must do.";
	close3;
}

1@wtgs,280,311,3	script(DISABLED)	Maristella Walter#ep21_rwi05	4_EP21_MARISTELLA,{
	if(get_instance_var("daily") == true){
		cutin "ep21_maristella01.png",1;
		mes "[Maristella Walter]";
		mes "There is nothing more to do here. Let's return to Alberta. Let's disembark from here.";
		close3;
	}
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "I witnessed it. The final moments of the Walter. They're will haven't faded one bit.";
	npctalk "Maristella Walter : I witnessed it. The final moments of the Walter. They're will haven't faded one bit.";
	next;
	mes "[Maristella Walter]";
	mes "As the captain said, this ship will disappear soon. Let's disembark before that happens.";
	npctalk "Maristella Walter : As the captain said, this ship will disappear soon. Let's disembark before that happens.";
	next;
	mes "[Maristella Walter]";
	mes "Let's disembark and move forward to do what we need to do.";
	npctalk "Maristella Walter : Let's disembark and move forward to do what we need to do.";
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "You need to come with terms on everything.";
	npctalk "Tan : You need to come with terms on everything.",instance_npcname("Tan#ep21_rw06");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Saying it like that makes it sound that we came all the way here just for me to sort out my feelings.";
	npctalk "Maristella Walter : Saying it like that makes it sound that we came all the way here just for me to sort out my feelings.";
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "That's what exactly what we did.";
	npctalk "Tan : That's what exactly what we did.",instance_npcname("Tan#ep21_rw06");
	next;
	cutin "ep21_iana03.png",1;
	mes "[Iana Operta]";
	mes "Mari, the vice-captain is worried about whether you've managed to sort out your emotions!";
	npctalk "Iana Operta : Mari, the vice-captain is worried about whether you've managed to sort out your emotions!",instance_npcname("Iana Operta#ep21_rwi05");
	next;
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "・・.";
	npctalk "Tan : ・・.",instance_npcname("Tan#ep21_rw06");
	next;
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "Is it really okay for us to leave captain here・・? Will we・・ never see him again?";
	npctalk "Yohan : Is it really okay for us to leave captain here・・? Will we・・ never see him again?",instance_npcname("Yohan#ep21_rw08");
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Yohan. You know just being able to speak with the captain today is an unbelievable stroke of luck.";
	npctalk "Maristella Walter : Yohan. You know just being able to speak with the captain today is an unbelievable stroke of luck.";
	next;
	mes "[Maristella Walter]";
	mes "We've already received so much. So much that it would be greedy to ask for anything more.";
	npctalk "Maristella Walter : We've already received so much. So much that it would be greedy to ask for anything more.";
	next;
	cutin "ep21_maristella02.png",1;
	mes "[Maristella Walter]";
	mes "The captain will be alright. You saw his expression before we left.";
	npctalk "Maristella Walter : The captain will be alright. You saw his expression before we left.";
	next;
	mes "[Maristella Walter]";
	mes "For the captain's sake, we can't stay here. We need to go out there.";
	npctalk "Maristella Walter : For the captain's sake, we can't stay here. We need to go out there.";
	next;
	cutin "ep21_maristella01.png",1;
	mes "[Maristella Walter]";
	mes "Alright, " + strcharinfo(0) + ", vice-commander, Iana, Yohan. Let's disembark now. Let's return to Alberta.";
	npctalk "Maristella Walter : Alright, vice-commander, Iana, Yohan. Let's disembark now. Let's return to Alberta.";
	if(isbegin_quest(16816) == 1){
		completequest 16816;
		setquest 16817;
	}
	if(get_instance_var("mps") < 13)
		instance_event("#EP21_GS_control","OnEvent",false);
	close3;

OnInstanceInit:
	questinfo QTYPE_QUEST,QMARK_NONE,"isbegin_quest(16816) == 1";
end;
}

1@wtgs,285,309,3	script(DISABLED)	Iana Operta#ep21_rwi05	4_EP21_IANA,{
	cutin "ep21_iana02.png",1;
	mes "[Iana Operta]";
	mes "Alright, alright, let's stay strong. Let's go back and drink something warm.";
	close3;
}

1@wtgs,282,313,3	script(DISABLED)	Tan#ep21_rw06	4_EP21_TAN,{
	cutin "ep21_tan01.png",1;
	mes "[Tan]";
	mes "Phew. The sea breeze is cold.";
	close3;
}

1@wtgs,277,312,3	script(DISABLED)	Yohan#ep21_rw08	4_EP21_YOHAN,{
	cutin "ep21_yohan01.png",2;
	mes "[Yohan]";
	mes "・・Captain. May you rest in peace.";
	close3;
}