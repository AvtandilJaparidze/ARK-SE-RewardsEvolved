#include "Commands.h"
#include "../Config/Config.h"
#include "../Rewards/Rewards.h"

void Commands::LoadCommands() const
{
	ArkApi::GetCommands().AddConsoleCommand(FString("RE.Reload"),
		std::bind(&Commands::ReloadConfigConsole, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	ArkApi::GetCommands().AddRconCommand(FString("RE.Reload"),
		std::bind(&Commands::ReloadConfigRcon, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	ArkApi::GetCommands().AddConsoleCommand(FString("RE.Reward"),
		std::bind(&Commands::GiveRewardConsole, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	ArkApi::GetCommands().AddRconCommand(FString("RE.Reward"),
		std::bind(&Commands::GiveRewardRcon, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void Commands::UnloadCommands() const
{
	ArkApi::GetCommands().RemoveConsoleCommand(FString("RE.Reload"));
	ArkApi::GetCommands().RemoveRconCommand(FString("RE.Reload"));

	ArkApi::GetCommands().RemoveConsoleCommand(FString("RE.Reward"));
	ArkApi::GetCommands().RemoveRconCommand(FString("RE.Reward"));
}

void Commands::ReloadConfigInternal() const
{
	ReadConfig();
}

void Commands::ReloadConfigConsole(APlayerController* APC, FString*, bool) const
{
	AShooterPlayerController* PC = static_cast<AShooterPlayerController*>(APC);

	try
	{
		this->ReloadConfigInternal();
	}
	catch (const std::exception& e)
	{
		const std::string ErrorMessage = fmt::format("Failed to reload config: {}", e.what());
		ArkApi::GetApiUtils().SendServerMessage(PC, FColorList::Red, *FString(ErrorMessage));
		Log::GetLog()->error(ErrorMessage);
		return;
	}

	ArkApi::GetApiUtils().SendServerMessage(PC, FColorList::Green, "Reloaded config");
}

void Commands::ReloadConfigRcon(RCONClientConnection* Connection, RCONPacket* Packet, UWorld*) const
{
	FString reply;

	try
	{
		this->ReloadConfigInternal();
	}
	catch (const std::exception& e)
	{
		const std::string ErrorMessage = fmt::format("Failed to reload config: {}", e.what());
		Log::GetLog()->error(ErrorMessage);
		reply = FString(ErrorMessage);
		Connection->SendMessageW(Packet->Id, 0, &reply);
		return;
	}

	reply = "Reloaded config";
	Connection->SendMessageW(Packet->Id, 0, &reply);
}

void Commands::GiveRewardInternal(const FString& CMD) const
{
	TArray<FString> Parsed;
	CMD.ParseIntoArray(Parsed, L" ", true);

	if (!Parsed.IsValidIndex(1))
	{
		throw std::runtime_error("Please provide a valid steam id");
	}

	if (!Parsed.IsValidIndex(2))
	{
		throw std::runtime_error("Please provide a valid reward id");
	}

	const uint64 SteamID = FCString::Atoi64(*Parsed[1]);

	AShooterPlayerController* PC = ArkApi::GetApiUtils().FindPlayerFromSteamId(SteamID);

	if (!PC)
	{
		throw std::runtime_error("Target player is not online");
	}

	const std::string RewardID = Parsed[2].ToString();

	RewardsMGR->GiveRewardFromConfig(PC, SteamID, RewardID);
}

void Commands::GiveRewardConsole(APlayerController* APC, FString* CMD, bool) const
{
	AShooterPlayerController* PC = static_cast<AShooterPlayerController*>(APC);

	try
	{
		this->GiveRewardInternal(*CMD);
	}
	catch (const std::exception& e)
	{
		const std::string ErrorMessage = fmt::format("Failed to give reward to player: {}", e.what());
		ArkApi::GetApiUtils().SendServerMessage(PC, FColorList::Red, *FString(ErrorMessage));
		Log::GetLog()->error(ErrorMessage);
		return;
	}

	ArkApi::GetApiUtils().SendServerMessage(PC, FColorList::Green, "Player rewarded!");
}

void Commands::GiveRewardRcon(RCONClientConnection* Connection, RCONPacket* Packet, UWorld*) const
{
	FString reply;

	try
	{
		this->GiveRewardInternal(Packet->Body);
	}
	catch (const std::exception& e)
	{
		const std::string ErrorMessage = fmt::format("Failed to give reward to player: {}", e.what());
		Log::GetLog()->error(ErrorMessage);
		reply = FString(ErrorMessage);
		Connection->SendMessageW(Packet->Id, 0, &reply);
		return;
	}

	reply = "Player rewarded!";
	Connection->SendMessageW(Packet->Id, 0, &reply);
}