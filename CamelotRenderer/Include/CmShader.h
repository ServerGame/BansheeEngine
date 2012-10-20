#pragma once

#include "CmPrerequisites.h"

namespace CamelotEngine
{
	/**
	 * @brief	Shader represents a collection of techniques. They are used in Materials,
	 * 			which can be considered as instances of a Shader. Multiple materials
	 * 			may share the same shader but provide different parameters to it.
	 * 			
	 *			Shader will always choose the first supported technique based on the current render
	 *			system, render manager and other properties. So make sure to add most important techniques
	 *			first so you make sure they are used if they are supported.
	 */
	class CM_EXPORT Shader
	{
	public:
		Shader(const String& name);

		void addTechnique(TechniquePtr technique);
		
		void removeTechnique(UINT32 idx);
		void removeTechnique(TechniquePtr technique);

		UINT32 getNumTechniques() const { return mTechniques.size(); }

	private:
		String mName;
		vector<TechniquePtr>::type mTechniques;
	};
}