/**
 * ExpansionSettings.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
 */

 modded class ExpansionSettings
 {
     static ref ScriptInvoker SI_FactionBank = new ScriptInvoker();
 
     override void Init()
     {
 #ifdef EXPANSIONTRACE
         auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "Init");
 #endif
 
        super.Init();

        #ifdef EXPANSIONMODHARDLINE
        //! Need to load hardline before FactionBank so we have access to reputation
        Init(ExpansionHardlineSettings, true);
        #endif

        Init(ExpansionFactionBankSettings, true);
     }
 
     void RPC_FactionBankSettings(PlayerIdentity sender, Object target, ParamsReadContext ctx)
     {
         Receive(ExpansionFactionBankSettings, ctx);
     }
 
     ExpansionFactionBankSettings GetFactionBank(bool checkLoaded = true)
     {
         return ExpansionFactionBankSettings.Cast(Get(ExpansionFactionBankSettings, checkLoaded));
     }
 };
 