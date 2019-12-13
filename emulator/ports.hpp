#ifndef __PORTS_HPP__
#define __PORTS_HPP__

#include "util.hpp"

/*
 * Structure représentant un composant utilisant un port.
 * - read_func : la fonction de lecture à utiliser lors de lecture sur le port utilisé : doit retourner un octet
 *               cette fonction est fournie par le composant qui veut utiliser un port
 * - write_func : la fonction de lecture à utiliser lors de l'écriture sur le port utilser : doit prendre un octet en argument
 *                cette fonction est fournie par le composant qui veut utiliser un port
 */

namespace E5150 { class Component; }

//TODO: insérer le numéro du port utilisé dans le struct le struct ?
struct PortInfos
{
	uint16_t portNum;
	E5150::Component* component;
};

class PORTS
{
	public:
		uint8_t read (const uint16_t port_number) const;
		void write(const uint16_t port_number, const uint8_t data);
		void connect (const PortInfos& portInfos);

	private:
		std::set<PortInfos> m_portDevices;
};

#endif