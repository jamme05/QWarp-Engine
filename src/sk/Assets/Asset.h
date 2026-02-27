/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Assets/Utils/Event.h>
#include <sk/Memory/Tracker/Tracker.h>
#include <sk/Misc/Hashing.h>
#include <sk/Misc/Smart_Ptrs.h>
#include <sk/Misc/StringID.h>
#include <sk/Misc/UUID.h>
#include <sk/Reflection/RuntimeClass.h>
#include <sk/Scene/Managers/EventManager.h>
#include <sk/Seralization/Serializable.h>
#include <sk/Seralization/SerializedObject.h>

#include <simdjson.h>

#include <any>
#include <filesystem>
#include <unordered_map>

namespace sk
{
	namespace Assets::Jobs
	{
		class cAsset_Worker;
	} // sk::Assets::Jobs::
	
	class cAsset_Manager;
	class cAsset;

	// Remember to update macro if changing this.
	constexpr auto kInvalid_Asset_Id = std::numeric_limits< uint64_t >::max();

	// Asset Metadata class
	class cAsset_Meta : public cShared_from_this< cAsset_Meta >
	{
		friend class cAsset_Manager;
		friend class cAsset_Ptr_Base;
		friend class Assets::Jobs::cAsset_Worker;
		
		static constexpr std::string_view kMetaExtension = "skmeta"; // = Skape Meta
	public:
		enum eFlags : uint16_t
		{
			kNone     = 0,

			// States:
			// If the asset is loaded.
			kLoaded   = 1 << 0,
			// If the asset is currently loading
			kLoading  = 1 << 1,
			// If the asset is currently unloading
			kUnloading = 1 << 2,

			// 3 - 5 reserved

			kDirty    = 1 << 6,
			
			// If the asset has a metadata file associated with it.
			kMetadata = 1 << 7,

			// If this asset shares a path with other assets. Ex: Multiple assets made from a single gltf file.
			kSharesPath = 1 << 8,
			
			// If this asset was manually created.
			kManualCreation = 1 << 9,
		};

		using dispatcher_t = Event::cDispatcherProxy< cAsset_Meta&, Assets::eEventType >;
		
		cAsset_Meta( std::string_view _name, type_info_t _asset_type );
		explicit cAsset_Meta( cSerializedObject& _object );
		~cAsset_Meta();
		cAsset_Meta( const cAsset_Meta& ) = delete;
		cAsset_Meta( cAsset_Meta&& ) = delete;
		cAsset_Meta& operator=( const cAsset_Meta& ) = delete;
		cAsset_Meta& operator=( cAsset_Meta&& ) = delete;

		auto& GetName() const { return m_name_; }
		auto& GetPath() const { return m_path_; }
		auto& GetAbsolutePath() const { return m_absolute_path_; }
		auto& GetExtension() const { return m_ext_; }
		
		// Saves any changes made to the asset.
		void Save();
		// Gets weather or not the asset is currently loading
		bool IsLoading() const;
		// Gets weather or not the asset is loaded
		bool IsLoaded() const;
		// If the asset is currently being loaded in, or is already loaded.
		bool IsLoadingOrLoaded() const;
		// If the asset is currently being unloaded
		bool IsUnloading() const;
		// Gets weather or not the asset has metadata
		bool HasMetadata() const;
		
		// Gets the UUID of this asset.
		auto GetUUID() const -> const cUUID&;
		// Gets the asset. Unsafe to be used directly as the state is controlled externally.
		auto GetAsset() const -> cAsset*;
		// Gets the asset if it is of a certain type
		template< reflected Ty >
		auto GetAsset() -> Ty*;
		// Gets the flags this asset has. Check eFlags for details.
		auto GetFlags() const -> uint16_t;
		// Gets the type info for the asset
		auto GetType () const -> type_info_t;
		// Gets the type info for the asset
		auto GetClass() const -> const iRuntimeClass*;
		// Gets the type hash of the asset
		auto GetHash () const -> type_hash;
		
		void Reload();

		// Adds a listener to get events on when the asset changes.
		// NOTE: Listeners do not load or unload the asset, and are only used to receive information.
		size_t AddListener( const dispatcher_t::event_t& _listener );

		// Adds a listener to get events on when the asset changes.
		// NOTE: Listeners do not load or unload the asset, and are only used to receive information.
		size_t AddListener( const dispatcher_t::weak_event_t& _listener );

		// Removes a listener from getting events on when the asset changes.
		// NOTE: Listeners do not load or unload the asset, and are only used to receive information.
		void RemoveListener( size_t _listener_id );

		// Removes a listener from getting events on when the asset changes.
		// NOTE: Listeners do not load or unload the asset, and are only used to receive information.
		void RemoveListener( const dispatcher_t::event_t& _listener );

		// Removes a listener from getting events on when the asset changes.
		// NOTE: Listeners do not load or unload the asset, and are only used to receive information.
		void RemoveListener( const dispatcher_t::weak_event_t& _listener );
		
		// Prevents the asset from being destroyed. Locks stack to allow for the asset to be used by multiple sources.
		void LockAsset();
		// Removes a lock from the asset.
		void UnlockAsset();

		void SetStoreIndex( size_t _value );
		auto GetStoreIndex() const -> size_t;

		void MarkDirty();
		bool IsDirty() const;

		// Always make sure to assign COMPLETED assets.
		// Providing an asset with incomplete information will be considered undefined behavior.
		// Made for internal usage. But can be used in case you want to manually create an asset.
		void setAsset( cAsset* _asset );

		auto Serialize() const -> cSerializedObject;

	private:
		void addReferrer   ( void* _source, const cWeak_Ptr< iClass >& _referrer );
		void removeReferrer( const void* _source, const cWeak_Ptr< iClass >& _referrer );
		void setPath( std::filesystem::path _path );

		void push_load_task( bool _load ) const;
		void push_save_task() const;

		// Requests a asset loader to post a asset loaded event to the specified listener.
		void dispatch_if_loaded( const dispatcher_t::listener_t& _listener );

		cStringID m_name_ = {};
		cStringID m_path_ = {};
		cStringID m_absolute_path_ = {};
		// File Extension
		cStringID m_ext_  = {};
		cUUID     m_uuid_ = cUUID::kInvalid;

		type_info_t m_asset_type_ = nullptr;
		// Do not write to directly, use setAsset instead.
		cAsset*     m_asset_      = nullptr;

		std::atomic_uint32_t m_asset_refs_;
		std::atomic_uint16_t m_lock_refs_ = 0;
		std::atomic_uint16_t m_flags_     = 0;

		size_t       m_store_index_ = 0;

		std::mutex   m_dispatcher_mutex_;
		dispatcher_t m_dispatcher_;

		// TODO: Make something more convenient for handling runtime asset metadata.
		std::unordered_map< str_hash, std::any > m_info_;

		// Nullptr is untrackable referrers.
		std::unordered_multimap< iClass*, void* > m_referrers_;
	};

	namespace Asset_Meta
	{
		using class_type = cAsset_Meta;
	} // Asset

	class cAsset;

	SK_CLASS( Asset ), public iSerializable
	{
		SK_CLASS_BODY( Asset )
		friend class cAsset_Manager;
		friend class cAsset_Meta;
	public:
		explicit cAsset( cSerializedObject& _object );

		[[ nodiscard ]] auto& GetMeta() const { return m_metadata_; }
		[[ nodiscard ]] auto& GetUUID() const { return m_uuid_; }
		auto Serialize() -> cSerializedObject override;
	protected:
		cAsset() = default;
	private:
		cUUID                    m_uuid_ = cUUID::kInvalid;
		cWeak_Ptr< cAsset_Meta > m_metadata_;
	};

	template< class In, class Out >
	using enable_if_asset_t = std::enable_if_t< std::is_base_of_v< cAsset, In >, Out >;
	template< class Ty >
	static constexpr bool is_valid_asset_v = std::is_base_of_v< cAsset, Ty >;

	template< reflected Ty >
	auto cAsset_Meta::GetAsset() -> Ty*
	{
		if( m_asset_->getClass().isDerivedFrom( Ty::getStaticClass() ) )
			return m_asset_;

		return nullptr;
	}

} // sk::

SK_DECLARE_CLASS( sk::Asset )

// Initializes data required for cAsset.
// The class created by this will be using the default naming (ex: Mesh becomes cMesh).
// Raw values can be accessed in sk::Assets::[Name], (ex sk::Assets::Mesh)
#define ASSET_PARENT_CLASS( AssetName, ... ) sk::cAsset
#define ASSET_PARENT_VALIDATOR( AssetName, ... ) sk::is_valid_asset_v< __VA_ARGS__ >
#define ASSET_PARENT_CREATOR_2( AssetName, ... ) AFTER_FIRST( __VA_ARGS__ )
#define ASSET_PARENT_CREATOR_1( AssetName, ... ) sk::cAsset
#define ASSET_PARENT_CREATOR( AssetName, ... ) CONCAT( ASSET_PARENT_CREATOR_, VARGS( __VA_ARGS__ ) ) ( AssetName, __VA_ARGS__ )
#define SK_ASSET_CLASS( AssetName, ... ) QW_RESTRICTED_CLASS( AssetName, ASSET_PARENT_CLASS, ASSET_PARENT_CREATOR, ASSET_PARENT_VALIDATOR, EMPTY __VA_OPT__( , __VA_ARGS__ ) )
