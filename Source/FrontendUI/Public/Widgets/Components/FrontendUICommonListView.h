#pragma once

#include "CoreMinimal.h"
#include "CommonListView.h"
#include "FrontendUICommonListView.generated.h"

UCLASS()
class FRONTENDUI_API UFrontendUICommonListView : public UCommonListView
{
	GENERATED_BODY()

protected:
	//~ Begin UCommonListView Interface
	virtual UUserWidget& OnGenerateEntryWidgetInternal(
		UObject* Item,
		TSubclassOf<UUserWidget> DesiredEntryClass,
		const TSharedRef<STableViewBase>& OwnerTable) override;
	virtual bool OnIsSelectableOrNavigableInternal(UObject* FirstSelectedItem) override;
	//~ End UCommonListView Interface

private:
	//~ Begin UWidget Interface
#if WITH_EDITOR	
	virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const override;
#endif
	//~ End UWidget Interface

	UPROPERTY(EditAnywhere, Category = "Frontend UI List View Settings")
	class UDataAsset_DataListEntryMapping* DataListEntryMapping;
};
