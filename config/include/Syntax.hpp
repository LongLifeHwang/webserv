/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Syntax.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: minsepar <minsepar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 19:12:12 by minsepar          #+#    #+#             */
/*   Updated: 2024/08/09 21:21:05 by minsepar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SYNTAX_HPP__
# define __SYNTAX_HPP__

#include <vector>

using namespace std;

class Syntax {
public:
    vector<int> tag;
    int required; // 0 = false, 1 = true, 2 = ...  
    Syntax(vector<int> t, int r);
};

#endif