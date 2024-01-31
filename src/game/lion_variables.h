
#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc) ;
#endif

//Tater Client Variables
MACRO_CONFIG_INT(ClRunOnJoinConsole, lc_run_on_join_console, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Whether to use run on join in chat or console")
MACRO_CONFIG_INT(ClRunOnJoinDelay, lc_run_on_join_delay, 2, 7, 50000, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Tick Delay before using run on join")

MACRO_CONFIG_INT(ClShowFrozenText, lc_frozen_tees_text, 0, 0, 2, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show how many tees in your team are currently frozen. (0 - off, 1 - show alive, 2 - show frozen)")
MACRO_CONFIG_INT(ClShowFrozenHud, lc_frozen_tees_hud, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show frozen tee HUD")
MACRO_CONFIG_INT(ClShowFrozenHudSkins, lc_frozen_tees_hud_skins, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Use ninja skin, or darkened skin for frozen tees on hud")

MACRO_CONFIG_INT(ClFrozenHudTeeSize, lc_frozen_tees_size, 15, 8, 20, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Size of tees in frozen tee hud. (Default : 15)")
MACRO_CONFIG_INT(ClFrozenMaxRows, lc_frozen_tees_max_rows, 1, 1, 6, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Maximum number of rows in frozen tee HUD display")
MACRO_CONFIG_INT(ClFrozenHudTeamOnly, lc_frozen_tees_only_inteam, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Only render frozen tee HUD display while in team")

MACRO_CONFIG_INT(ClPingNameCircle, lc_nameplate_ping_circle, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Shows a circle next to nameplate to indicate ping")

MACRO_CONFIG_INT(ClSpecmenuID, lc_spec_menu_ID, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Shows player IDs in spectate menu")

MACRO_CONFIG_INT(ClLimitMouseToScreen, lc_limit_mouse_to_screen, 0, 0, 2, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Limit mouse to screen boundries")



//Anti Latency Tools
MACRO_CONFIG_INT(ClFreezeUpdateFix, lc_freeze_update_fix, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "(WIP) Will change your skin faster when you enter freeze. ")
MACRO_CONFIG_INT(ClRemoveAnti, lc_remove_anti, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Removes some amount of antiping & player prediction in freeze")
MACRO_CONFIG_INT(ClUnfreezeLagTicks, lc_remove_anti_ticks, 5, 0, 20, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "The biggest amount of prediction ticks that are removed")
MACRO_CONFIG_INT(ClUnfreezeLagDelayTicks, lc_remove_anti_delay_ticks, 100, 0, 200, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "How many ticks it takes to remove the maximum prediction after being frozen")
MACRO_CONFIG_INT(ClAdjustRemovedDelay, lc_adjust_removed_delay, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Add delay back if you are not currently in freeze (helps up down saves)")
MACRO_CONFIG_INT(ClUnpredOthersInFreeze, lc_unpred_others_in_freeze, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Dont predict other players if you are frozen")

MACRO_CONFIG_INT(ClPredMarginInFreeze, lc_pred_margin_in_freeze, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "enable changing prediction margin while frozen")
MACRO_CONFIG_INT(ClPredMarginInFreezeAmount, lc_pred_margin_in_freeze_amount, 15, 0, 2000, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Set what your prediction margin while frozen should be")



MACRO_CONFIG_INT(ClShowOthersGhosts, lc_show_others_ghosts, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show ghosts for other players in their unpredicted position")
MACRO_CONFIG_INT(ClSwapGhosts, lc_swap_ghosts, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show predicted players as ghost and normal players as unpredicted")
MACRO_CONFIG_INT(ClHideFrozenGhosts, lc_hide_frozen_ghosts, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Hide Ghosts of other players if they are frozen")

MACRO_CONFIG_INT(ClPredGhostsAlpha, lc_pred_ghosts_alpha, 100, 0, 100, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Alpha of predicted ghosts (0-100)")
MACRO_CONFIG_INT(ClUnpredGhostsAlpha, lc_unpred_ghosts_alpha, 50, 0, 100, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Alpha of unpredicted ghosts (0-100)")
MACRO_CONFIG_INT(ClRenderGhostAsCircle, lc_render_ghost_as_circle, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Render Ghosts as circles instead of tee")

//MACRO_CONFIG_INT(ClHookLineSize, lc_hook_line_width, 0, 0, 20, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Adjustable hookline width, set to 0 for old default rendering")

MACRO_CONFIG_INT(ClShowCenterLines, lc_show_center, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Draws lines to show the center of your screen/hitbox")

MACRO_CONFIG_INT(ClShowSkinName, lc_skin_name, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Shows skin names in nameplates, good for finding missing skins")

//MACRO_CONFIG_INT(ClFreeGhost, lc_freeghost, 0, 0, 1, CFGFLAG_CLIENT , "")

//Outline Variables
MACRO_CONFIG_INT(ClOutline, lc_outline, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Draws outlines")
MACRO_CONFIG_INT(ClOutlineEntities, lc_outline_in_entities, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Only show outlines in entities")
MACRO_CONFIG_INT(ClOutlineFreeze, lc_outline_freeze, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Draws outline around freeze and deep")
MACRO_CONFIG_INT(ClOutlineUnFreeze, lc_outline_unfreeze, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Draws outline around unfreeze and undeep")
MACRO_CONFIG_INT(ClOutlineTele, lc_outline_tele, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Draws outline around teleporters")
MACRO_CONFIG_INT(ClOutlineSolid, lc_outline_solid, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Draws outline around hook and unhook")
MACRO_CONFIG_INT(ClOutlineWidth, lc_outline_width, 5, 1, 16, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "(1-16) Width of freeze outline")
MACRO_CONFIG_INT(ClOutlineAlpha, lc_outline_alpha, 50, 0, 100, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "(0-100) Outline alpha")
MACRO_CONFIG_INT(ClOutlineAlphaSolid, lc_outline_alpha_solid, 100, 0, 100, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "(0-100) Outline solids alpha")
MACRO_CONFIG_COL(ClOutlineColorSolid, lc_outline_color_solid, 0, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Solid outline color") //0 0 0
MACRO_CONFIG_COL(ClOutlineColorFreeze, lc_outline_color_freeze, 0, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Freeze outline color") //0 0 0
MACRO_CONFIG_COL(ClOutlineColorTele, lc_outline_color_tele, 0, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Tele outline color") //0 0 0
MACRO_CONFIG_COL(ClOutlineColorUnfreeze, lc_outline_color_unfreeze, 0, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Unfreeze outline color") //0 0 0

//Indicator Variables
MACRO_CONFIG_COL(ClIndicatorAlive, lc_indicator_alive, 255, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Color of alive tees in player indicator")
MACRO_CONFIG_COL(ClIndicatorFreeze, lc_indicator_freeze, 65407, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Color of frozen tees in player indicator")
MACRO_CONFIG_COL(ClIndicatorSaved, lc_indicator_dead, 0, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Color of tees who is getting saved in player indicator")
MACRO_CONFIG_INT(ClIndicatorOffset, lc_indicator_offset, 42, 16, 200, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "(16-128) Offset of indicator position")
MACRO_CONFIG_INT(ClIndicatorOffsetMax, lc_indicator_offset_max, 100, 16, 200, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "(16-128) Max indicator offset for variable offset setting")
MACRO_CONFIG_INT(ClIndicatorVariableDistance, lc_indicator_variable_distance, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Indicator circles will be further away the further the tee is")
MACRO_CONFIG_INT(ClIndicatorMaxDistance, lc_indicator_variable_max_distance, 1000, 500, 7000, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Maximum tee distance for variable offset")
MACRO_CONFIG_INT(ClIndicatorRadius, lc_indicator_radius, 4, 1, 16, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "(1-16) indicator circle size")
MACRO_CONFIG_INT(ClIndicatorOpacity, lc_indicator_opacity, 50, 0, 100, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Opacity of indicator circles")
MACRO_CONFIG_INT(ClPlayerIndicator, lc_player_indicator, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show radial indicator of other tees")
MACRO_CONFIG_INT(ClPlayerIndicatorFreeze, lc_player_indicator_freeze, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Only show frozen tees in indicator")
MACRO_CONFIG_INT(ClIndicatorTeamOnly, lc_indicator_inteam, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Only show indicator while in team")
MACRO_CONFIG_INT(ClIndicatorTees, lc_indicator_tees, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show tees instead of circles")

//Bind Wheel
MACRO_CONFIG_INT(ClResetBindWheelMouse, lc_reset_bindwheel_mouse, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Reset position of mouse when opening bindwheel")



MACRO_CONFIG_INT(ClWhiteFeet, lc_white_feet, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Render all feet as perfectly white base color")
MACRO_CONFIG_STR(ClWhiteFeetSkin, lc_white_feet_skin, 255, "x_ninja", CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Base skin for white feet")

MACRO_CONFIG_INT(ClMiniDebug, lc_mini_debug, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show position and angle")

MACRO_CONFIG_INT(ClNotifyWhenLast, lc_last_notify, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Notify when you are last")
MACRO_CONFIG_STR(ClNotifyWhenLastText, lc_last_notify_text, 64, "Last!", CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Text for last notify")
MACRO_CONFIG_COL(ClNotifyWhenLastColor, lc_last_notify_color, 256, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Color for last notify")

MACRO_CONFIG_INT(ClRenderCursorSpec, lc_cursor_in_spec, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Render your gun cursor when spectating in freeview")

MACRO_CONFIG_INT(ClRenderNameplateSpec, lc_render_nameplate_spec, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Render nameplates when spectating")

MACRO_CONFIG_INT(ClApplyProfileSkin, lc_profile_skin, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Apply skin in profiles")
MACRO_CONFIG_INT(ClApplyProfileName, lc_profile_name, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Apply name in profiles")
MACRO_CONFIG_INT(ClApplyProfileClan, lc_profile_clan, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Apply clan in profiles")
MACRO_CONFIG_INT(ClApplyProfileFlag, lc_profile_flag, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Apply flag in profiles")
MACRO_CONFIG_INT(ClApplyProfileColors, lc_profile_colors, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Apply colors in profiles")
MACRO_CONFIG_INT(ClApplyProfileEmote, lc_profile_emote, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Apply emote in profiles")

// Voting
MACRO_CONFIG_INT(ClVoteAuto, lc_vote_auto, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Activate auto vote")
MACRO_CONFIG_INT(ClVoteDefaultAll, lc_vote_default, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Default vote everybody (0:yes,1:no)")
MACRO_CONFIG_INT(ClVoteDefaultFriend, lc_vote_default, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Default vote friends (0:yes,1:no,3:Default)")
MACRO_CONFIG_INT(ClVoteDontShow, lc_vote_show, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show only votes where your vote counts")
MACRO_CONFIG_INT(ClVoteDontShowFriends, lc_vote_show_friends, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Show only votes where your vote counts exclude friends")

// Auto Verify
MACRO_CONFIG_INT(ClAutoVerify, lc_auto_verify, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Auto verify")

// Rainbow
MACRO_CONFIG_INT(ClRainbow, lc_rainbow, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Turn on rainbow client side")
MACRO_CONFIG_INT(ClRainbowOthers, lc_rainbow_others, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Turn on rainbow client side for others")
MACRO_CONFIG_INT(ClRainbowMode, lc_rainbow_mode, 1, 1, 4, CFGFLAG_CLIENT | CFGFLAG_LSAVE, "Rainbow mode (1: rainbow, 2: pulse, 3: darkness)")

//AAAAAAA
MACRO_CONFIG_INT(ClAmIFrozen, EEEfrz, 0, 0, 1, CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(ClFreezeTick, EEEfrztk, 0, 0, 9999, CFGFLAG_CLIENT, "")