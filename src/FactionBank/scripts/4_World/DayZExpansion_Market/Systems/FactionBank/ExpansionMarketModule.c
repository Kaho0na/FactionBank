/**
 * ExpansionMarketModule.c
 *
 * DayZ Expansion Mod - Faction Bank by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

modded class ExpansionMarketModule
{
    static ref ScriptInvoker SI_FactionBankMenuCallback = new ScriptInvoker();

    ref array<ref ExpansionMarketATM_Data> m_ATMData;



    override void OnInit()
    {
        super.OnInit();

        Expansion_RegisterServerRPC("RPC_RequestPlayerFactionData");
        Expansion_RegisterClientRPC("RPC_SendPlayerFactionData");
        Expansion_RegisterServerRPC("RPC_RequestDonateMoney");
		Expansion_RegisterClientRPC("RPC_ConfirmDonateMoney");
    }
    
    
	// ------------------------------------------------------------
	// Expansion RequestDonateMoney
	// ------------------------------------------------------------	
	void RequestDonateMoney(int amount)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!GetGame().IsDedicatedServer())
		{
			auto rpc = Expansion_CreateRPC("RPC_RequestDonateMoney");
			rpc.Write(amount);
			rpc.Expansion_Send(true);
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestDonateMoney
	// ------------------------------------------------------------
	private void RPC_RequestDonateMoney(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
			return;

		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_RequestDonateMoney - Could not get amount!");
			return;
		}
		
		Exec_RequestDonateMoney(amount, senderRPC);
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestDonateMoney
	// ------------------------------------------------------------
	private void Exec_RequestDonateMoney(int amount, PlayerIdentity ident)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!ident)
		{
			Error("ExpansionMarketModule::Exec_RequestDonateMoney - Could not get player identity!");
			return;
		}

		PlayerBase player = PlayerBase.GetPlayerByUID(ident.GetId());
		if (!player)
		{
			Error("ExpansionMarketModule::Exec_RequestDonateMoney - Could not get player base enity!");
			return;
		}
				
		ExpansionMarketATM_Data data = GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionMarketModule::Exec_RequestDonateMoney - Could not find player atm data!");
			
			return;
		}

		data.RemoveMoney(amount);
		data.Save();
        int factionBankBalance = GetExpansionSettings().GetFactionBank().FactionBankBalance;
		factionBankBalance += amount;
		GetExpansionSettings().GetFactionBank().FactionBankBalance = factionBankBalance;
		GetExpansionSettings().GetFactionBank().Save();
		ExpansionLogATM(string.Format("Player \"%1\" (id=%2) has Donated %3 to the Faction Bank.", ident.GetName(), ident.GetId(), amount));
		ConfirmDonateMoney(amount, ident, data, factionBankBalance);
	}
		
	// ------------------------------------------------------------
	// Expansion ConfirmDonateMoney
	// ------------------------------------------------------------	
	void ConfirmDonateMoney(int amount, PlayerIdentity ident, ExpansionMarketATM_Data data, int factionBankBalance)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif
		auto rpc = Expansion_CreateRPC("RPC_ConfirmDonateMoney");
		rpc.Write(amount);
		rpc.Write(data);
		rpc.Write(factionBankBalance);
		rpc.Expansion_Send(true, ident);
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmDonateMoney
	// ------------------------------------------------------------
	private void RPC_ConfirmDonateMoney(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
			return;

		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_ConfirmDonateMoney - Could not get amount!");
			return;
		}
		
		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionMarketModule::RPC_ConfirmDonateMoney - Could not get player ATM data!");
			return;
		}

		int factionBankBalance;
		if (!ctx.Read(factionBankBalance))
		{
			Error("ExpansionMarketModule::RPC_ConfirmDonateMoney - Could not get faction bank balance!");
			return;
		}

		
		Exec_ConfirmDonateMoney(amount, data, factionBankBalance);
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmDonateMoney
	// ------------------------------------------------------------
	private void Exec_ConfirmDonateMoney(int amount, ExpansionMarketATM_Data data, int factionBankBalance)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		SI_FactionBankMenuCallback.Invoke(amount, data, factionBankBalance); // <-- Pass both
	}
	
    
    // ------------------------------------------------------------
    // Expansion RequestPlayerFactionData
    // ------------------------------------------------------------
    void RequestPlayerFactionData()
    {
#ifdef EXTRACE
        auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

        if (!GetGame().IsDedicatedServer())
        {
            auto rpc = Expansion_CreateRPC("RPC_RequestPlayerFactionData");
            rpc.Expansion_Send(true);
        }
    }


    // ------------------------------------------------------------
    // Expansion OpenFactionMenu
    // ------------------------------------------------------------
    bool OpenFactionMenu()
    {
        if (!MoneyCheck())
            return false;

        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("FactionBankMenu");

        return true;
    }
    
    // ------------------------------------------------------------
    // Expansion RPC_RequestPlayerFactionData
    // ------------------------------------------------------------
    private void RPC_RequestPlayerFactionData(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
    {
#ifdef EXTRACE
        auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif
        
        SendPlayerFactionData(senderRPC);
    }

    void SendPlayerFactionData(PlayerIdentity ident)
    {
	#ifdef EXTRACE
			auto trace = EXTrace.Start(EXTrace.MARKET, this);
	#endif

		if (!ident)
		{
			Error("ExpansionMarketModule::SendPlayerFactionData - Could not get sender indentity!");
			return;
		}
		
		ExpansionMarketATM_Data data = GetPlayerATMData(ident.GetId());
		int FactionBankBalance = GetExpansionSettings().GetFactionBank().FactionBankBalance;
		if (!data)
		{
			Error("ExpansionMarketModule::SendPlayerFactionData - Could not get ExpansionMarketATM_Data!");
			return;
		}
			
		auto rpc = Expansion_CreateRPC("RPC_SendPlayerFactionData");
		rpc.Write(data);
		rpc.Write(FactionBankBalance);
		rpc.Expansion_Send(true, ident);
    }
    // ------------------------------------------------------------
    // Expansion RPC_SendPlayerFactionData
    // ------------------------------------------------------------
    private void RPC_SendPlayerFactionData(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
    {
    #ifdef EXTRACE
            auto trace = EXTrace.Start(EXTrace.MARKET, this);
    #endif

    if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
        return;

    ExpansionMarketATM_Data data;
    if (!ctx.Read(data))
    {
        Error("ExpansionMarketModule::RPC_SendPlayerFactionData - Could not get ExpansionMarketATM_Data!");
        return;
    }
	int FactionBankBalance;
	if (!ctx.Read(FactionBankBalance)){
		Error("ExpansionMarketModule::RPC_SendPlayerFactionData - Could not get FactionBankBalance!");
		return;
	}
	
	if (!GetGame().IsDedicatedServer())
	{
		if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
			return;

		if (!OpenFactionMenu())
			return;
	}
    
    Exec_SendPlayerFactionData(data, FactionBankBalance);
    }

    // ------------------------------------------------------------
    // Expansion Exec_SendPlayerFactionData
    // ------------------------------------------------------------
    private void Exec_SendPlayerFactionData(ExpansionMarketATM_Data data, int FactionBankBalance)
    {
#ifdef EXTRACE
        auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

        if (!OpenFactionMenu())
            return;

			SetPlayerFactionData(data, FactionBankBalance);
    }
    
	// ------------------------------------------------------------
	// Expansion SetPlayerFactionData
	// ------------------------------------------------------------
	void SetPlayerFactionData(ExpansionMarketATM_Data data, int FactionBankBalance)
	{
	#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
	#endif

		SI_ATMMenuInvoker.Invoke(data, FactionBankBalance); // <-- Pass both
	}
	
}