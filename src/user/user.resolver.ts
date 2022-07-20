import { Args, Mutation, Query, Resolver } from '@nestjs/graphql';
import { CreateUserInput, FindUserInput, User } from './user.schema';
import { UserService } from './user.service';

@Resolver(() => User)
export class UserResolver {
    constructor(private userService: UserService){}

    @Query(() => [User])
    async users(){
        return this.userService.findMany();
    }

    @Query(() => User)
    async user(@Args('input') {_id}: FindUserInput){
        return this.userService.findById(_id);
    }

    @Mutation(() => User)
    async createUser(@Args('input') user: CreateUserInput){
        return this.userService.createUser(user);
    }
}
