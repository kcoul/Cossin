/**
    ===============================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any internal version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) 2019 ElandaSunshine
    ===============================================================
    
    @author Elanda
    @file   ParameterAttachmentUtil.cpp
    @date   17, June 2020
    
    ===============================================================
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <variant>

template<class TargetObject, class Attachment>
struct AttachmentEntry
{
    using TargetType     = TargetObject;
    using AttachmentType = Attachment;
};

template<class ...List>
struct AttachmentList
{
private:
    template<class>         struct isPairCheck : std::false_type {};
    template<class ...Args> struct isPairCheck<AttachmentEntry<Args...>> : std::true_type {};
    template<class Pair>    static constexpr bool isPair = isPairCheck<Pair>::value;
    
    //==================================================================================================================
    template<class TargetType, class FirstPair, class ...Others>
    struct AttachmentOfBase
    {
        using Type = std::conditional_t<std::is_base_of_v<typename FirstPair::TargetType, TargetType>,
                                        typename FirstPair::AttachmentType,
                                        typename AttachmentOfBase<TargetType, Others...>::Type>;
    };

    template<class TargetType>
    struct AttachmentOfBase<TargetType, std::void_t<>>
    {
        using Type = std::void_t<>;
    };

public:
    static_assert((isPair<List> && ...), R"(Type "List" may only contain AttachmentEntry objects)");
    
    //==================================================================================================================
    template<template<class...> class PackageType>
    using UnpackTargets = PackageType<typename List::TargetType...>;
    
    template<template<class...> class PackageType>
    using UnpackAttachments = PackageType<typename List::AttachmentType...>;
    
    template<class TargetType>
    using AttachmentOf = typename AttachmentOfBase<TargetType, List..., std::void_t<>>::Type;
    
    //==================================================================================================================
    using VariantType = UnpackAttachments<std::variant>;
    
    //==================================================================================================================
    template<class TargetType>
    static constexpr bool hasAttachmentFor = (std::is_base_of_v<typename List::TargetType, TargetType> || ...);
    
    //==================================================================================================================
    class Array
    {
    public:
        template<class Target, class ...Args>
        void attach(juce::RangedAudioParameter &par, Target &target, Args &&...args)
        {
            static_assert(hasAttachmentFor<Target>, "No attachment type for the given target type was registered");
            
            if constexpr (hasAttachmentFor<Target>)
            {
                attachments.emplace_back(std::make_unique<VariantType>(std::in_place_type<AttachmentOf<Target>>, par,
                                                                       target, std::forward<Args>(args)...));
            }
        }
    private:
        std::vector<std::unique_ptr<VariantType>> attachments;
    };
};

template<class ...Entries>
using DefaultAttachmentList = AttachmentList<
    AttachmentEntry<juce::Button,   juce::ButtonParameterAttachment>,
    AttachmentEntry<juce::Slider,   juce::SliderParameterAttachment>,
    AttachmentEntry<juce::ComboBox, juce::ComboBoxParameterAttachment>,
    Entries...
>;
