
int __pthread_spin_lock(pthread_spinlock_t *lock){
 
	int val = 0;
	if(__glibc_likely (atomic_exchange_acquire (lock, 1) == 0)) return 0;
	/*
	把lock 的值跟1 作對調： 如果lock 一開始值是0, exchange 這個函式會回傳舊的lock 值（0）。
	，意思是這個lock目前沒有被拿走，可以馬上使用。所以如果它回傳0，則要取用的
	process 就可以拿走lock了，不用進入下面的while 去spin 。
	如果lock 值是1，代表有其他process已經通過上述的功能拿走這個lock了，（就是比它優先執行	到這個函數的其他process），exchange 函式回傳lock舊值(也就是1)，所以他要去下面的	while 	等待(spin)。
	*/

	do{
		do{
			/*TODO Back-Off*/
			atomic_spin_nop(); 
			/*相當於pause，目的是不要讓CPU在while裡面太操*/

			val = atomic_load_relaxed(lock);
			/*把lock值load 到val裡面(read)*/

		}while(val != 0);
		/*當這次的lock不是0，即lock還在其他人手上沒有被釋放出來，就繼續spin*/
		//上述是加強效能（執行較快）

	/*
	  能跳出裡面的while代表被釋放了，但有其他processes也讀到它們val 是0，所以下面要一起
	atomic確認。最快執行到下面atomic_compare_exchange_weak_acquire 函數 的process就能搶	到這個lock，之後其他讀到val == 0的processes 執行atomic_compare_exchange_weak_acquire 	就只會回傳false (lock 舊的值，被最快的那個搶先改成1了，所以lock 跟val （0）對他們來說不	一樣），就又被卡在while裡面了。
	*/
	}while(!atomic_compare_exchange_weak_acquire(lock, &val, 1));
	/* atomic_compare_exchange_weak_acquire(*obj, *except, desire):
	當*obj == *except時，將obj 改成desire，回傳true; else則將*except改成*obj,回傳false 
	設計要將local 的val 改成1 (上述紅字的部份)是因為這樣就能讓其他要等待的processes進入到內	部while(val != 0)迴圈去等(雖然之後val 會有do重新去讀lock，應該不會影響)，效能較高*/	
	
	return 0;
	/*拿到這個lock了*/
}

int __pthread_spin_unlock(pthread_spinlock_t *lock){
	atomic_store_release(lock,0);
	/*釋放這個lock(使用atomic operation)。release 則是GUN在關於釋放操作時的函數*/
}
