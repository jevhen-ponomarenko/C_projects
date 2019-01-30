//
//  Autocorrector.cpp
//  Autocorrect
//
//  Created by Yevgen Ponomarenko on 1/23/19.
//  Copyright © 2019 Yevgen Ponomarenko. All rights reserved.
//

#include "Autocorrector.hpp"
#include <fstream>
#include <algorithm>
#include <atomic>

autocorrector::autocorrector(){
    m_dictionary = loadWords(pathToDictionary);
}

const size_t autocorrector::getDistance2(const std::string & originalWord, const std::string & compareTo) const {
    const size_t _maxOffset = originalWord.length();
    const size_t m(originalWord.length());
    const size_t n(compareTo.length());
    
    if( m==0 ) return n;
    if( n==0 ) return m;
    
    int c = 0;
    int offset1 = 0;
    int offset2 = 0;
    int lcs = 0;
    while ((c + offset1 < originalWord.length())
           && (c + offset2 < compareTo.length()))
    {
        if (originalWord[c + offset1] == compareTo[c + offset2]) lcs++;
        else
        {
            offset1 = 0;
            offset2 = 0;
            if (originalWord[c] == compareTo[c])
            {
                c++;
                continue;
            }
            for (int i = 1; i < _maxOffset; i++)
            {
                if ((c + i < originalWord.length())
                    && (originalWord[c + i] == compareTo[c]))
                {
                    offset1 = i;
                    break;
                }
                if ((c + i < compareTo.length())
                    && (originalWord[c] == compareTo[c + i]))
                {
                    offset2 = i;
                    break;
                }
            }
        }
        c++;
    }
    return (originalWord.length() + compareTo.length())/2 - lcs;
}

const size_t autocorrector::getDistance(const std::string & originalWord, const std::string & compareTo) const{
    const size_t m(originalWord.length());
    const size_t n(compareTo.length());
    
    if( m==0 ) return n;
    if( n==0 ) return m;
    
    size_t *costs = new size_t[n + 1];
    
    for(size_t k = 0; k <= n; k++) costs[k] = k;
    
    size_t i = 0;
    for (std::string::const_iterator it1 = originalWord.begin(); it1 != originalWord.end(); ++it1, ++i)
    {
        costs[0] = i+1;
        size_t corner = i;
        
        size_t j = 0;
        for (std::string::const_iterator it2 = compareTo.begin(); it2 != compareTo.end(); ++it2, ++j)
        {
            size_t upper = costs[j+1];
            if(*it1 == *it2)
            {
                costs[j + 1] = corner;
            }
            else
            {
                size_t t(upper < corner ? upper : corner);
                costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
            }
            
            corner = upper;
        }
    }
    
    size_t result = costs[n];
    delete [] costs;
    
    return result;
}
/**
 * Building dictionary char to set, keys of the map are letters of alphabet;
 * value of the key is set of words (to prevent duplicites).
 *
 */

std::map<char,std::set<std::string>> autocorrector::loadWords(const std::string path){
    std::map<char,std::set<std::string>> letterToWordMap;
    std::string line;
    std::fstream file(path);
    
    while (std::getline(file, line)) {
        auto it = letterToWordMap.find(line[0]);
        //if keys is in the dictionary
        if (it == letterToWordMap.end()) {
            std::set<std::string> newSet;
            newSet.insert(line);
            letterToWordMap.insert(std::pair<char,std::set<std::string>>(line[0], newSet));
        }else{
            (*it).second.insert(line);
        }
        
    }
    return letterToWordMap;
}

autocorrector::vectorOfWords autocorrector::correctWord(std::string &word){
    char firstLetter = word[0];
    auto it = m_dictionary.find(firstLetter);
    std::vector<std::string> possibleWords;
    if (it == m_dictionary.end()) {
        //throw exeption
        ;
    }else{
        //iterating through the set of words
        auto it2 = (*it).second.begin();
        size_t maxDistance = 3;
        
        while (it2 != (*it).second.end()) {
            size_t actualDistance = getDistance(word, *it2);
            if (actualDistance == 0){
                possibleWords.clear();
                return possibleWords;
            }
            else if (actualDistance < maxDistance) {
                maxDistance = actualDistance;
                possibleWords.clear();
                possibleWords.push_back(*it2);
            }
            else if(actualDistance == maxDistance){
                possibleWords.push_back(*it2);
            }
            it2++;
        }
    }
    return possibleWords;
}
autocorrector::vectorOfWords autocorrector::correctWord2(std::string &word){
    char firstLetter = word[0];
    auto it = m_dictionary.find(firstLetter);
    std::vector<std::string> possibleWords;
    if (it == m_dictionary.end()) {
        //throw exeption
        ;
    }else{
        //iterating through the set of words
        auto it2 = (*it).second.begin();
        size_t maxDistance = word.length();
        
        while (it2 != (*it).second.end()) {
            size_t actualDistance = getDistance2(word, *it2);
            if (actualDistance == 0){
                possibleWords.clear();
                return possibleWords;
            }
            else if (actualDistance < maxDistance) {
                maxDistance = actualDistance;
                possibleWords.clear();
                possibleWords.push_back(*it2);
            }
            else if(actualDistance == maxDistance){
                possibleWords.push_back(*it2);
            }
            it2++;
        }
    }
    return possibleWords;
}

void autocorrector::correctWordParallel(std::string &word, std::promise<vectorOfWords> promise){
    char firstLetter = word[0];
    auto it = m_dictionary.find(firstLetter);
    std::vector<std::string> possibleWords;
    if (it == m_dictionary.end()) {
        //throw exeption
        ;
    }else{
        //iterating through the set of words
        auto it2 = (*it).second.begin();
        size_t maxDistance = 3;
        
        while (it2 != (*it).second.end()) {
            size_t actualDistance = getDistance2(word, *it2);
            if (actualDistance == 0){
                possibleWords.clear();
                break;
            }
            else if (actualDistance < maxDistance) {
                maxDistance = actualDistance;
                possibleWords.clear();
                possibleWords.push_back(*it2);
            }
            else if(actualDistance == maxDistance){
                possibleWords.push_back(*it2);
            }
            it2++;
        }
    }
    promise.set_value(possibleWords);
}

