import { Field, ID, InputType, ObjectType } from "@nestjs/graphql";
import { Prop, Schema, SchemaFactory } from "@nestjs/mongoose";
import mongoose from "mongoose";

export type UserDocument = User & mongoose.Document;

@Schema()
@ObjectType()
export class User {
    @Field(() => ID)
    _id: number;

    @Prop({required: true})
    @Field()
    name: string;

    @Prop({required: true})
    @Field()
    email: string;

    @Prop({required: true})
    @Field()
    phone: string;

    @Prop({required: true})
    @Field()
    gender: string;
}

export const UserSchema = SchemaFactory.createForClass(User);

@InputType()
export class CreateUserInput {
    @Field()
    name: string;

    @Field()
    email: string;

    @Field()
    phone: string;

    @Field()
    gender: string;
}

@InputType()
export class FindUserInput {
    @Field()
    _id: string;
}