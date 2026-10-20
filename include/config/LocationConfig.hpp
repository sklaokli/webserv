/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:44:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <map>
#include <string>
#include <vector>

class LocationConfig {
public:
	typedef std::map<std::string, std::string> CgiMap;

	struct StatusCodes {
		static const int MIN = 300;
		static const int MAX = 399;
		static const int DEFAULT = 302;
	};

	LocationConfig();
	explicit LocationConfig(const std::string&);
	LocationConfig(const LocationConfig&);
	LocationConfig& operator=(const LocationConfig&);
	~LocationConfig();

	void dump(size_t = 0) const;

	const std::string& getPath() const;
	const std::vector<std::string>& getAllowedMethods() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	bool getAutoindex() const;
	int getRedirectCode() const;
	const std::string& getRedirectUrl() const;
	bool getUploadEnable() const;
	const std::string& getUploadStore() const;
	const CgiMap& getCgiExt() const;
	size_t getClientMaxBodySize() const;

	void setPath(const std::string&);
	void setAllowedMethods(const std::vector<std::string>&);
	void addAllowedMethod(const std::string&);
	void setRoot(const std::string&);
	void setIndex(const std::string&);
	void setAutoindex(bool);
	void setRedirect(int, const std::string&);
	void setUploadEnable(bool);
	void setUploadStore(const std::string&);
	void addCgiExt(const std::string&, const std::string&);
	void setClientMaxBodySize(size_t);

	bool isMethodAllowed(const std::string&) const;
	bool hasRedirect() const;
	bool hasCgi(const std::string&) const;
	std::string getCgiHandler(const std::string&) const;

private:
	std::string _path;
	std::vector<std::string> _allowedMethods;
	std::string _root;
	std::string _index;
	bool _autoindex;
	int _redirectCode;
	std::string _redirectUrl;
	bool _uploadEnable;
	std::string _uploadStore;
	CgiMap _cgiExt;
	size_t _clientMaxBodySize;
};

#endif
