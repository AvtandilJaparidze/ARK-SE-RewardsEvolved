#include "../Include/RewardsAPI.h"
#include "../Rewards/Rewards.h"

bool RewardsAPI::GiveReward(AShooterPlayerController* PC, const std::string& RewardID)
{
	try
	{
		if (!PC)
		{
			throw std::runtime_error("PlayerController is null");
		}

		const uint64 SteamID = ArkApi::GetApiUtils().GetSteamIdFromController(PC);

		if (SteamID == NULL)
		{
			throw std::runtime_error("Failed to get SteamID from PlayerController");
		}

		RewardsMGR->GiveRewardFromConfig(PC, SteamID, RewardID);
		return true;
	}
	catch (const std::exception& e)
	{
		Log::GetLog()->error("Failed to give reward to player: {}", e.what());
		return false;
	}

	return false;
}