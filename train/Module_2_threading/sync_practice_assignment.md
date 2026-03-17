## Question
You are developing a thread-safe banking application where multiple threads may attempt to withdraw or deposit money from the same account at the same time.

Explain how you would use mutexes, locks, or condition variables to ensure the account balance remains accurate.

Describe at least one technique for preventing deadlock when multiple resources are involved.

Discuss how you would design the account class to ensure it is thread-safe, while still maintaining good performance.

Finally, explain the difference between a situation that requires synchronization and one that does not require synchronization in this context.

###  Attempt 1
### Answer
Typically banking operations would be part of the list.

Create a new account (check if the account already exists)

credit (adding to value)

debit (deduction in value)

transfer (addition to one and deduction in other)

close the account (withdraw full amount to zero and mark the account disable)

For the account balance to remain accurate, all these operations for each account need to be serialised. Semantics of each operation are:

Create -- need to lock the mutex at the bank level, so no two creation requests can overlap.

Credit, Debit need to lock a mutex at account level. 

transfer needs to lock the mutex for both the accounts and subsequently the same account mutex will be locked by credit on one account and debit on the other account.

close the account also need to lock the mutex at bank level so it can be resolved in a serial fashion.

Based on this we need below setup.

bank_mutex -- a classic mutex at the back class level, which is locked for create and close account api calls.

account_mutex -- a recursive mutex to support multiple locks on the same mutex on the transfer and credit/debit operations. This is the case where we have multiple resources getting locked together.

Since credit/debit/transfer only lock the mutex at account level, operations on different accounts can happen in parallel.

This indicates all operations need synchronisation though the level of locking is different in different contexts.

At present there seem to be no need for condition_variables in this context as I cannot envision the scenario where a thread will wait for a condition after having acquired a lock on all the various mutexes.

## Feedback
##### Synchronization Mechanisms Explanation ----- Status: 4/5 points

Your explanation of synchronization mechanisms is quite detailed. You effectively described the use of mutexes at both the bank and account levels, which is crucial for maintaining account balance accuracy. However, the explanation could benefit from more examples or potential issues that might arise, such as deadlock scenarios or performance bottlenecks. Keep up the good work and consider exploring these areas further to enhance your understanding.

5 points
Excellent: Provides a comprehensive explanation of mutexes, locks, or condition variables, with clear examples of how they maintain account balance accuracy, and addresses potential issues or considerations.

##### Deadlock Prevention Technique ----- Status: 2/5 points

You mentioned the use of recursive mutexes to handle multiple locks, which is a good start. However, the explanation lacks depth and clarity on how this technique effectively prevents deadlock. Consider providing more detailed examples or discussing other strategies like lock ordering or timeout mechanisms to strengthen your understanding and application of deadlock prevention techniques.

2 points
Basic: Describes a technique for preventing deadlock with minimal detail or clarity.

4 points
Proficient: Describes an effective technique for preventing deadlock, with some explanation of its application and effectiveness.

5 points
Excellent: Provides a detailed and clear description of a technique for preventing deadlock, including examples and explanation of its effectiveness and application.

Status: 4/5 points
4/5 points
Thread-safe Account Class Design

You discussed the design of a thread-safe account class with a focus on using mutexes at different levels. While this shows consideration for performance, the discussion could be more comprehensive with specific examples or techniques to maintain performance. Consider exploring additional strategies or examples to enhance your design approach.

How useful is this feedback?
0 points
Inadequate: Does not discuss the design of a thread-safe account class or discussion is incorrect.

2 points
Basic: Discusses the design of a thread-safe account class with minimal detail or consideration of performance.

4 points
Proficient: Discusses the design of a thread-safe account class with consideration for performance, but lacks depth or specific examples.

5 points
Excellent: Provides a comprehensive discussion on designing a thread-safe account class, including specific techniques and examples for maintaining performance.

Status: 4/5 points
4/5 points
Synchronization Necessity Explanation

Your explanation of synchronization necessity is somewhat clear, as you mentioned that all operations need synchronization. However, it would be beneficial to provide more examples and context to differentiate between situations that require synchronization and those that do not. This will help solidify your understanding and application of synchronization in various scenarios.

How useful is this feedback?
0 points
Inadequate: Does not explain the difference between situations that require synchronization and those that do not, or explanation is incorrect.

2 points
Basic: Explains the difference between situations that require synchronization and those that do not, but lacks detail or clarity.

4 points
Proficient: Explains the difference between situations that require synchronization and those that do not, with some examples and clarity.

5 points
Excellent: Provides a clear and detailed explanation of the difference between situations that require synchronization and those that do not, with relevant examples and context.