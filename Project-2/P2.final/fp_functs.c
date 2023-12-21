/* 
 * Name: Adilet Kuroda 
 * GNumber: 01253384
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fp.h"


/*Prototypes */
int check_sign(float *val); // Determines if the given number is positive or negative 
int get_exponent(float *val); // extract the exponent part of the given float 
int get_frac(float * val); // extacts the frac part of the float with rounding to nearest even 
int get_fp_gmu_sign(fp_gmu val); // gets the sign of a fp_fmu
int get_fp_gmu_exponent(fp_gmu val); // extracts the exponent bit repsentation of fp_gmu
int get_fp_gmu_frac(fp_gmu val); // // extract bit representation of fp_fmu frac portion 
float get_mantissa(int frac); // converts the frac portion of fp_gmu to mantissa 
int check_infi_NaN_zero(fp_gmu source1); // determines if the parameter is zero, infinity or NaN or else. 
int balance_E(int sign_1, int sign_2, float *M_1, float * M_2); // Balances E for addition 
fp_gmu add_two_fp_gmu_values(fp_gmu source1, fp_gmu source2); // adds two non NaN, zero, infinity fp_gmu floats 
fp_gmu multiply_two_fp_gmu_values(fp_gmu source1, fp_gmu source2); // multiplies two non NaN, zero, infinity fp_gmu floats

/* Converts parameter float to fp_gmu (12 bit float) and returns it. 
 * if provided number is too small, it returns -0 or 0. 
 * Returns a fp_fmu (12 bit float)*/
fp_gmu compute_fp(float val){
    fp_gmu ret = 0;
    int sign = check_sign(&val); // returns the s

    if (val == 0){
      return ret | sign; 
    }
    int exponent = get_exponent(&val);
    val -= 1;
    int frac = get_frac(&val);
    if (frac > 63){
      exponent += 1;
      frac = 0;
    }
    if(exponent <= 0){  // handles underflow(denormilized)
        ret |= sign;
        return ret; 
    }

    if (exponent >= 31){ // handles overflow 
        if(sign > 0){
            ret = 0x3F; // Negative infinity 
        }
        else{
            ret = 0x1F; // Positive infinity 
        }
        return ret << 6; 
    }
    exponent <<= 6;
    ret |= sign;
    ret |= exponent;
    ret |= frac;
    return ret;
       
}
/* Obtains fraction part of the fp_gmu float rounding to nearest even 
 * Return int representing frac part of the fp_gmu */
int get_frac(float * val){
    int i = 0;
    for(i = 0; i < 6; i++){
        *val *=2;
    }
    int ret = (int)*val;
    *val -= ret;
    if (*val == 0.5){  
        if ((ret % 2) == 1){
            return (ret+1);
        }
        ret;
    }
    if (*val > 0.5){
        return (ret +1);
    }
    return ret;
}

/* Determines if the given argument is negative or positive number
 * Also change the parameter to positive if it is negative and shift the sign 11 bit to m
 * make sure the sign bit is on right spot 
 * Return greater than 1 if negative else, 0 */
int check_sign(float *val){
    int ret = 0;
    if (*val < 0)
    {
        *val *= -1;
        ret = 1;
        return ret << 11;
    }
    return ret;
}

/*Get the exponent part of the floating point. Exponent = E + Bias
 * return exponent part of fp_gmu float*/
int get_exponent(float *val){
if (*val == 0){  // ensures number is not 0
        return -1;
    }
    int bias = 15;  // 2^x -1 
    int E = 0;
    if ((*val >= 1) && (*val < 2)){ 
        return (E + bias);
    }
    if (*val < 1){  // 
        while(*val < 1){
           *val *= 2;
           E -= 1; 
        }
        return (E + bias); 
    }
    else{
        while(*val >= 2){
            *val /=2;
            E += 1;
        }
        return (E + bias);
    }    
}


/* Converts fp_gmu representation to C float. Follows following rules for given input
 * If your input fp_gmu is Infinity, return the constant INFINITY
 * If your input fp_gmu is -Infinity, return the constant -INFINITY
 * If your input fp_gmu is NaN, return the constant NAN
 * Returns: float value represented by our fp_gmu encoding
 */
float get_fp(fp_gmu val) {
  if (val == 0x800 || val == 0){        // Need to clarify 
    return 0;
  }
  float ret = 0;
  int bias = 15, E = 0, i = 0;
  int sign = get_fp_gmu_sign(val);
  int exponent = get_fp_gmu_exponent(val);
  int frac = get_fp_gmu_frac(val);
  if (exponent >= 31){
    if (frac == 0){
      return ((sign > 0) ? -INFINITY : INFINITY);
    }
    return NAN;
  }
  ret = frac; 
  for ( i = 0; i < 6; i++){ // turns it to fraction 
    ret /= 2;
  }
  E = exponent - bias; // calculates the power 
  ret = ret + 1; // adds mantissa 
  if (E < 0){ // ensures E = 0 where sign* float * 2^0
    while (E != 0){
      E++;
      ret /=2;
    }
  }
  else{
   while (E != 0){
     E--;
     ret *= 2;
   } 
  }
  if (sign > 0){
    ret *= -1;
  }
  return ret;
}
 

/* Obtains the sign of a fp_gmu 
 * Return 0 if positive. Otherwise returns a number greater than 0*/
int get_fp_gmu_sign(fp_gmu val){
  int ret = 1;
  ret <<= 11;
  ret = ret & val;
  return ret;
}
/* Obtains exponent part of the fp_gmu (which is 5 bit)
 * Returns integer representing exponent part of fp_gmu */
int get_fp_gmu_exponent(fp_gmu val){
  int ret = 0x1F;
  ret <<= 6;
  ret = ret & val;
  ret >>= 6;
  return ret;
}
/* Obtains the fraction part of the fp_gmu float 
 * return integer representing fraction part of the float */
int get_fp_gmu_frac(fp_gmu val){
  int ret = 0x3F;
  ret = ret & val;
  return ret;
}

/* This function mulitplies two fp_gmu floating points following following 
 * Arithmetic Rules:
 * If either argument is NaN, return your fp_gmu NaN
 * If you multiply any non-Zero, non NaN value to ∞ or − ∞,
 *      return ∞ or − ∞ as appropriate based on the sign rules for multiplication
 * If you multiply 0 or -0 by non NaN, non Infinity, return either
 *        0 or -0 (based on the normal sign rules for multiplication)
 * If you multiply 0 * -0, or vice versa, you will return either 0 or-0
 * For any other arithmetic operation which would result in a zero, return 0.
 * 
 * Mulitplication is implemented using following precedure:
 *   Xor the signs: S = S1 ^ S2
 *   Add the exponents: E = E1 + E2
 *   Multiply the Frac Values: M = M1 * M2
 *   If M is not in a valid range for normalized, adjust M and E.
z
 * Returns The multiplication result in our fp_gmu representation
 */
fp_gmu mult_vals(fp_gmu source1, fp_gmu source2) {
  fp_gmu ret = 0;
  /* check_infi_NaN_zero return 1 for zero, 2 for NAN  3 for Infinity and Zero for everything else;*/
  int status_1 = check_infi_NaN_zero(source1);  
  int status_2 = check_infi_NaN_zero(source2);
  if ((status_1 == 0) && (status_2 == 0)){
    return multiply_two_fp_gmu_values(source1, source2); 
  }
  fp_gmu Not_Number = 0x7C2; 
  fp_gmu neg_infinity = 0xFC0;
  fp_gmu pos_infinity = 0x7C0;
  fp_gmu neg_zero = 0x800;
  fp_gmu pos_zero = 0;
  int sign_1 = get_fp_gmu_sign(source1);
  int sign_2 = get_fp_gmu_sign(source2);
  int sign_final = sign_1 ^ sign_2;
  if ((status_1 == 2) || (status_2 == 2)){
    return Not_Number;
  }
  if ((status_1 == 3) || (status_2 == 3)){ // check if both are infinity 
    if ((status_1 == 1) || (status_2 == 1)){
      return Not_Number;
    }
    return ((sign_final > 0)? neg_infinity: pos_infinity);
  }
  return ((sign_final > 0)? neg_zero:pos_zero);
}
/* This function multiplies two non zero, non infinity, non NaN fp_gmu floats 
 * Following following procedure: 
 *   Xor the signs: S = S1 ^ S2
 *   Add the exponents: E = E1 + E2
 *   Multiply the Frac Values: M = M1 * M2
 *   If M is not in a valid range for normalized, adjust M and E.
 * Returns result of fp_gmu float multiplication*/
fp_gmu multiply_two_fp_gmu_values(fp_gmu source1, fp_gmu source2){
  int bias = 15, i = 0;
  int sign_1 = get_fp_gmu_sign(source1);
  int sign_2 = get_fp_gmu_sign(source2);
  int sign_final = sign_1 ^ sign_2;

  int exponent_1 = get_fp_gmu_exponent(source1);
  int exponent_2 = get_fp_gmu_exponent(source2);
  int E_final = (exponent_1 + exponent_2) - 2*bias;

  int frac_1 = get_fp_gmu_frac(source1);
  int frac_2 = get_fp_gmu_frac(source2);
  
  float M_1 = get_mantissa(frac_1);
  float M_2 = get_mantissa(frac_2);
  float M_final = M_1 * M_2;
  if (M_final >= 2){
    while(M_final >= 2){
      E_final++;
      M_final /= 2;
    }
  }
  if(M_final < 1){
    while (M_final < 1){
      E_final--;
      M_final *= 2;
    }
  }
  M_final -= 1;
  int frac_final = get_frac(&M_final);
  int exponent_final = E_final + bias;
  fp_gmu Not_Number = 0x7C2; 
  fp_gmu neg_infinity = 0xFC0;
  fp_gmu pos_infinity = 0x7C0;
  fp_gmu neg_zero = 0x800;
  fp_gmu pos_zero = 0;

  if (exponent_final >=  31){
    return ((sign_final > 0) ? neg_infinity:pos_infinity);
  }
  
  if (exponent_final == 0){
    return ((sign_final > 0)? neg_zero:pos_zero);
  }

  fp_gmu ret = 0;
  ret |= sign_final;
  ret |= frac_final;
  exponent_final <<= 6;
  ret |= exponent_final;
  return ret;
}

/*Converts the integer to floating point and adds one to get the whole mantissa
 * Returns float representing mantissa of a actual float
 */
float get_mantissa(int frac){
  float ret = frac;
  int i = 0;
  for (i = 0; i< 6; i++){
    ret /= 2;
  }
  ret++;
  return ret;
}

/* Function calculate sum of two pf_gmu floats using following rules: 
 * Adjusting the numbers to get the same exponent E
 * Add the two adjusted Mantissas: M = M1 + M2
 * Adjust M and E so that it is in the correct range [1, 2) for Normalized
 * Also following following arithmetic rules: 
 * If you add ∞ + −∞ (either order), return your fp_gmu NaN.
 * If you add any value (non NaN, -∞) to ∞, return ∞.
 * If you add any value (non NaN, ∞) to −∞, return −∞.
 * If you add -0 + -0, you will return -0.
 * If you add -0 + 0, or vice versa, you will return 0.
 * If you add 0 or -0 to any value that is non NaN, non Infinity,
 * you will return that other value.
 * Returns The addition result in our fp_gmu representation. 
 */
fp_gmu add_vals(fp_gmu source1, fp_gmu source2) {
  fp_gmu ret = 0;
  /* check_infi_NaN_zero return 1 for zero, 2 for NAN  3 for Infinity and Zero for everything else;*/
  int status_1 = check_infi_NaN_zero(source1);  
  int status_2 = check_infi_NaN_zero(source2);
  if ((status_1 == 0) && (status_2 == 0)){
    return add_two_fp_gmu_values(source1, source2); 
  }
  fp_gmu Not_Number = 0x7C2; // Check for NaN 
  fp_gmu neg_infinity = 0xFC0;
  fp_gmu pos_infinity = 0x7C0;
  fp_gmu neg_zero = 0x800;
  fp_gmu pos_zero = 0;
  int sign_1 = get_fp_gmu_sign(source1);
  int sign_2 = get_fp_gmu_sign(source2);
  if ((status_1 == 2) || (status_2 == 2)){
    return Not_Number;
  } 
  if ((status_1 == 3) && (status_2 == 3)){ // check if both are infinity 
    return Not_Number;
  }
  if (status_1 == 3){ // if only first parameter is infinity 
    return (sign_1 > 0)? neg_infinity: pos_infinity; // returns apropriate sign infinity 
  }
  if (status_2 == 3){  // if only second parameter is infinity 
    return (sign_2 > 0)? neg_infinity: pos_infinity;
  }
  if ((status_1 == 1) && (status_2 == 1)){ // if both parameters are 0
    if ((sign_1 > 0) && (sign_2 > 0)){
      return neg_zero;
    }
    return pos_zero;
  }
  if (status_1 == 1){  // if the first parameter is zero while second is not 
    return source2;
  }
  return source1; // if second parameter is zero while first one is not
}
/* Ensures the exponent of two numbers are same for addition and 
 * adjusts the mantissas accordingly. 
 * returns final adjusted exponent E as integer value 
 */
int balance_E(int E_1, int E_2, float *M_1, float * M_2){
 if (E_1 != E_2){
    if (E_1 > E_2){
      while( E_1 != E_2){
        E_2++;
       *M_2 /= 2;
      }
    }
    else{
      while( E_1 != E_2){
        E_1++; 
       *M_1 /= 2;
      }
    }
  }
  return E_1; 
}
/* Determines if the number in infinity, NaN or zero, 
 * Return 1 for zero, 2 for NAN  3 for Infinity and Zero for everything else;
 */
int check_infi_NaN_zero(fp_gmu source1){
  int ret = 0;
  int sign = get_fp_gmu_sign(source1);
  int frac = get_fp_gmu_frac(source1);
  int expo = get_fp_gmu_exponent(source1);

  if (expo == 0){
    return 1;
  }
  if (expo == 31){
    return (frac == 0) ? 3:2;

  }

  return ret;
}
/* Adds two num zero, nor infinity and non NAN fpu_gmu numbers together. 
 * Return sum of two fp_gmu number as fp_gmu. 
 */
fp_gmu add_two_fp_gmu_values(fp_gmu source1, fp_gmu source2){
int bias = 15;
    int E_1 = get_fp_gmu_exponent(source1) - bias;
    int E_2 = get_fp_gmu_exponent(source2) - bias;

    int sign_1 = get_fp_gmu_sign(source1);
    int sign_2 = get_fp_gmu_sign(source2);

    float M_1 = get_mantissa(get_fp_gmu_frac(source1));
    float M_2 = get_mantissa(get_fp_gmu_frac(source2));
    
    int E_final = balance_E(E_1, E_2, &M_1, &M_2);
    if (sign_1 > 0){
      M_1 *= -1;
    }
    if (sign_2 > 0){
      M_2 *= -1;
    }
    float M_final = M_1 + M_2;
    int sign_final = check_sign(&M_final);
    if (M_final == 0){
      return 0;
    }
    int E_temp = get_exponent(&M_final) - bias;
    E_final += E_temp;
    
    fp_gmu Not_Number = 0x7C2; // Check for NaN 
    fp_gmu neg_infinity = 0xFC0;
    fp_gmu pos_infinity = 0x7C0;
    fp_gmu neg_zero = 0x800;
    fp_gmu pos_zero = 0;
    int exponent_final = E_final + bias;
    if (exponent_final == 0){
      return ((sign_final > 0)? neg_zero: pos_zero);
    }
    if (exponent_final >= 31){
      return ((sign_final >= 0)? neg_infinity: pos_infinity);
    }

    exponent_final <<= 6;
    M_final -= 1;
    int frac_final = get_frac(&M_final);
    fp_gmu ret = 0;
    ret |= sign_final;
    ret |= frac_final;
    ret |= exponent_final;
    return ret;
}


