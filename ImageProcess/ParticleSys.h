// ParticleSys.h: interface for the CParticleSys class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLESYS_H__753BFFB8_17DD_4EC7_B86F_202533B12A24__INCLUDED_)
#define AFX_PARTICLESYS_H__753BFFB8_17DD_4EC7_B86F_202533B12A24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CParticleSys  
{
public:
	CParticleSys();
	virtual ~CParticleSys();

};

template <size_t size, class InitializePolicy, class ActionPolicy, class ParticleType>
class ParticleGroup
{
public : 
	InitializePolicy d_InitializePolicy;
	ActionPolicy d_ActionPolicy;

	explicit ParticleGroup() throw() : d_nCurrentCount(0) {}
	~ParticleGroup() throw() {}

	inline void Clear() throw() {
		d_nCurrentCount = 0;
	}
	inline const ParticleType* GetParticles() const throw() {
		if ( ParticlesCount() == 0 ) {
			return 0;
		}
		return d_arrParticles;
	}
	inline const size_t MaxParticles() const throw() {
		return size;
	}
	inline const size_t ParticlesCount() const throw() {
		return d_nCurrentCount;
	}
	void Emit(const size_t& i_nAmount, const Point3D& i_ptPosition) {
		size_t nAmount = i_nAmount;
		//  exceed limit?
		if ( ( ParticlesCount() + nAmount ) > MaxParticles() ) {
			nAmount = MaxParticles() - ParticlesCount();
		}
		if ( nAmount > 0 ) {
			//  create the particles
			size_t nCnt = d_nCurrentCount;
			d_nCurrentCount += nAmount;
			for(; nCnt < d_nCurrentCount; ++nCnt) {
				d_arrParticles[nCnt].d_ptPos = i_ptPosition;
				d_InitializePolicy(d_arrParticles[nCnt]);
			}
		}
	}
	void Update() throw() {
		d_ActionPolicy.PrepareAction();
		//  kill off all dead particles
		for(size_t nCnt = 0; nCnt < d_nCurrentCount; ) {
			d_ActionPolicy(d_arrParticles[nCnt]);
			if ( d_arrParticles[nCnt].d_nLife <= 0 ) {
				//  dead, move last particle to this particle
				d_arrParticles[nCnt] = d_arrParticles[d_nCurrentCount - 1];
				//  decrease particle count.
				--d_nCurrentCount;
			} else {
				//  move to next particle
				++nCnt;
			}
		}
	}
private : 
	ParticleType d_arrParticles[size];
	size_t d_nCurrentCount;
};  //  end of class ParticleGroup



#endif // !defined(AFX_PARTICLESYS_H__753BFFB8_17DD_4EC7_B86F_202533B12A24__INCLUDED_)
