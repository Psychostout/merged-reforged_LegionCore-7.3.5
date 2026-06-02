-- Q34392 : Marks Shadowmoon (Nord, guid 105850) et Bleeding Hollow (Sud, guid 105853)
UPDATE gameobject_template SET ScriptName = 'gob_mark_of_tanaan'
  WHERE entry IN (233056, 233057);

-- Q34393 : Sub-marks Burning Blade (guid 105817), Shattered Hand (guid 105818), Blackrock (guid 105813)
UPDATE gameobject_template SET ScriptName = 'gob_q34393_mark'
  WHERE entry IN (229598, 229599, 229600);

-- Q34393 : Stasis Rune (guid 105801) → crédit 78333 + scène Gul'dan (spell 163807)
UPDATE gameobject_template SET ScriptName = 'gob_stasis_rune'
  WHERE entry = 233104;
