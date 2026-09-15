/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:29:05 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <map>
#include <string>
#include <vector>

class LocationConfig {
public:
	LocationConfig();
	LocationConfig(const LocationConfig& other);
	LocationConfig& operator=(const LocationConfig& other);
	~LocationConfig();

	const std::string& getPath() const;
	const std::vector<std::string>& getAllowedMethods() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	bool getAutoindex() const;
	int getRedirectCode() const;
	const std::string& getRedirectUrl() const;
	bool getUploadEnable() const;
	const std::string& getUploadStore() const;
	const std::map<std::string, std::string>& getCgiExt() const;
	size_t getClientMaxBodySize() const;

	void setPath(const std::string& path);
	void setAllowedMethods(const std::vector<std::string>& methods);
	void addAllowedMethod(const std::string& method);
	void setRoot(const std::string& root);
	void setIndex(const std::string& index);
	void setAutoindex(bool autoindex);
	void setRedirect(int code, const std::string& url);
	void setUploadEnable(bool enable);
	void setUploadStore(const std::string& store);
	void addCgiExt(const std::string& ext, const std::string& handler);
	void setClientMaxBodySize(size_t size);

	bool isMethodAllowed(const std::string& method) const;
	bool hasRedirect() const;
	bool hasCgi(const std::string& ext) const;
	std::string getCgiHandler(const std::string& ext) const;

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
	std::map<std::string, std::string> _cgiExt;
	size_t _clientMaxBodySize;
};

#endif
