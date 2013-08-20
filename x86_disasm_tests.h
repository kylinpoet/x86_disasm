/*
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#pragma once

void disas_test1(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be);
void disas_test2_2op(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be, int value2_must_be);
void disas_test2_1op(TrueFalseUndefined x64, const unsigned char* code, disas_address adr, const char *must_be, int value1_must_be);

/* vim: set expandtab ts=4 sw=4 : */
