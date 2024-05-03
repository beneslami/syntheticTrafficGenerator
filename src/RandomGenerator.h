//
// Created by Ben on 6/30/21.
//

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <iostream>
#include <random>
#include <map>
#include "random_utils.hpp"

namespace RandomGenerator {
    static std::random_device rd;
    static std::mt19937 mt_rng(rd());

    class BernoulliDistribution{
    public:
        BernoulliDistribution(){

        }

        BernoulliDistribution(double p) {
            dist = new std::bernoulli_distribution(p);
        }

        ~BernoulliDistribution() {
        }

        void SetProbability(double p) {
            dist = new std::bernoulli_distribution(p);
        }

        bool Generate() {
            return (*dist)(mt_rng);
        }

    private:
        std::bernoulli_distribution* dist;
    };

    class UniformDistribution{
    public:
        UniformDistribution(){

        }

        UniformDistribution(int min, int max) {
            dist = new std::uniform_real_distribution<double>(min, max);
        }

        ~UniformDistribution() {
        }

        void SetMinMax(int min, int max) {
            dist = new std::uniform_real_distribution<double>(min, max);
        }

        double Generate() {
            return (*dist)(mt_rng);
        }
    private:
        std::uniform_real_distribution<double>* dist;
    };

    class NormalDistribution{
    public:
        NormalDistribution(float mean, float std){
            dist = new std::normal_distribution<float>(mean, std);
        }

        ~NormalDistribution(){
        }

        float generate(){
            return (*dist)(mt_rng);
        }

    private:
        std::normal_distribution<float>* dist;

    };

    class ExponentialDistribution{
    public:
        ExponentialDistribution(std::exponential_distribution<double>* dist){
            this->dist = dist;
        }

        ExponentialDistribution(double lambda){
            _lambda = lambda;
        }

        double Generate(){
            std::exponential_distribution<double> dist(_lambda);
            return dist(mt_rng);
        }

    private:
        double _lambda;
        std::exponential_distribution<double>* dist;
    };

    class PoissonDistribution{
    public:
        PoissonDistribution(std::poisson_distribution<int>* dist){
            this->dist = dist;
        }

        PoissonDistribution(double lambda){
            _lambda = lambda;
        }
        int Generate(){
            std::poisson_distribution<int> dist(_lambda);
            return dist(mt_rng);
        }
    private:
        int _lambda;
        std::poisson_distribution<int>* dist;
    };

    class CustomDistribution{
    public:
        CustomDistribution(){

        }

        CustomDistribution(std::map<int, int> bytes){
            std::map<int, int>::iterator it;
            for(it = bytes.begin(); it != bytes.end(); ++it){
                distribution[it->first] = it->second;
            }
            int range = 0;
            for(it = distribution.begin(); it != distribution.end(); ++it){
                range += it->second;
            }
            for(it = distribution.begin(); it != distribution.end(); ++it){
                pdf[it->first] = double(it->second)/range;
            }
            double prev = 0;
            std::map<int, double>::iterator it2;
            for(it2 = pdf.begin(); it2 != pdf.end(); ++it2){
                cdf[it2->first] = it2->second + prev;
                prev = cdf[it2->first];
            }
        }

        ~CustomDistribution(){

        }

        void show_pdf(){
            std::map<int, double>::iterator it;
            for(it = this->pdf.begin(); it != this->pdf.end(); ++it){
                std::cout << it->first << ": " << it->second << std::endl;
            }
        }

        void show_cdf(){
            std::map<int, double>::iterator it;
            for(it = this->cdf.begin(); it != this->cdf.end(); ++it){
                std::cout << it->first << ": " << it->second << std::endl;
            }
        }

        double Generate(){
            std::uniform_real_distribution<double> dist = std::uniform_real_distribution<double>(0, 1);
            double prob = dist(mt_rng);
            for(std::map<int, double>::iterator it = cdf.begin(); it != cdf.end(); ++it){
                if(prob <= it->second){
                    return it->first;
                }
            }
            return -1;
        }

        bool isEmpty(){
            if(cdf.empty()){
                return true;
            }
            else{
                return false;
            }
        }

        int size(){
            return this->cdf.size();
        }

        bool is_in(int state){
            std::map<int, int>::iterator it = distribution.find(state);
            bool flag;
            if(it != distribution.end()){
                flag = 1;
            }
            else{
                flag = 0;
            }
            return flag;
        }

        int smallest(){
            int minimum = 1024;
            std::map<int, int>::iterator it;
            for(it = distribution.begin(); it != distribution.end(); ++it){
                if(it->first <= minimum){
                    minimum = it->first;
                }
            }
            return minimum;
        }

    private:
        std::map<int, int> distribution;
        std::map<int, double> cdf;
        std::map<int, double> pdf;
    };
};


#endif
